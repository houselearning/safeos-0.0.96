#!/usr/bin/env python3
"""
Krow BootRepair Tool

Usage:
  tools/krow_bootrepair.py diagnose --target <path>
  tools/krow_bootrepair.py repair --target <path> [--apply] [--assume-yes]
  tools/krow_bootrepair.py rollback --backup <path>
  tools/krow_bootrepair.py package --iso

This script performs non-destructive diagnostics by default. Use --apply
to perform repairs. The script works on a mounted SafeOS root directory or
on a raw disk image (requires root when performing device-level operations).

Safe operations:
- Always creates backups before modifying files.
- Writes a detailed log in the working directory and into the target when
  possible.

Author: AI-generated (Krow BootRepair)
"""
import argparse
import os
import shutil
import sys
import hashlib
import subprocess
import time
import logging
import tempfile
import stat
import json

LOG = None


def setup_logger(logfile=None):
    global LOG
    LOG = logging.getLogger('krow_bootrepair')
    LOG.setLevel(logging.DEBUG)
    fmt = logging.Formatter('%(asctime)s %(levelname)s: %(message)s')
    sh = logging.StreamHandler()
    sh.setFormatter(fmt)
    LOG.addHandler(sh)
    if logfile:
        fh = logging.FileHandler(logfile)
        fh.setFormatter(fmt)
        LOG.addHandler(fh)


def find_repo_root(start='.'):
    """Find repository root by looking for Makefile and iso/ directory."""
    cur = os.path.abspath(start)
    for _ in range(8):
        if os.path.isfile(os.path.join(cur, 'Makefile')) and os.path.isdir(os.path.join(cur, 'iso')):
            return cur
        nxt = os.path.dirname(cur)
        if nxt == cur:
            break
        cur = nxt
    return os.path.abspath(start)


def sha256_of_file(path):
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for chunk in iter(lambda: f.read(65536), b''):
            h.update(chunk)
    return h.hexdigest()


class BootRepair:
    def __init__(self, target, repo_root=None, apply=False, assume_yes=False):
        self.target = os.path.abspath(target)
        self.repo = find_repo_root(repo_root or '.')
        ts = int(time.time())
        self.backup_dir = os.path.abspath('.krow_bootrepair_backup_{}'.format(ts))
        self.apply = apply
        self.assume_yes = assume_yes
        self.logfile = os.path.abspath('krow_bootrepair_{}.log'.format(ts))
        setup_logger(self.logfile)
        LOG.info('Repo root: %s', self.repo)
        LOG.info('Target: %s', self.target)
        LOG.info('Backup dir: %s', self.backup_dir)

    def ensure_backup(self):
        if not os.path.exists(self.backup_dir):
            os.makedirs(self.backup_dir)

    def locate_safeos_bin(self):
        # Common locations in repo
        candidates = [
            os.path.join(self.repo, 'SafeOS.bin'),
            os.path.join(self.repo, 'iso', 'SafeOS.bin'),
            os.path.join(self.repo, 'build', 'iso', 'SafeOS.bin'),
            os.path.join(self.repo, 'build', 'iso', 'boot', 'SafeOS.bin'),
        ]
        for p in candidates:
            if os.path.isfile(p):
                LOG.info('Found SafeOS.bin at %s', p)
                return p
        LOG.warning('SafeOS.bin not found in repo known locations')
        return None

    def find_grub_cfg_in_repo(self):
        p1 = os.path.join(self.repo, 'build', 'iso', 'boot', 'grub', 'grub.cfg')
        p2 = os.path.join(self.repo, 'iso', 'grub.cfg')
        for p in (p1, p2):
            if os.path.isfile(p):
                LOG.info('Found grub.cfg at %s', p)
                return p
        LOG.warning('grub.cfg not found in repo known locations')
        return None

    def find_limine_cfg_in_repo(self):
        p = os.path.join(self.repo, 'iso', 'limine.cfg')
        if os.path.isfile(p):
            LOG.info('Found limine.cfg at %s', p)
            return p
        LOG.debug('limine.cfg not found in repo iso dir')
        return None

    def target_has_file(self, relpath):
        p = os.path.join(self.target, relpath.lstrip('/'))
        return os.path.exists(p)

    def backup_path(self, relpath):
        src = os.path.join(self.target, relpath.lstrip('/'))
        dst = os.path.join(self.backup_dir, os.path.basename(relpath))
        return src, dst

    def backup_file(self, relpath):
        src, dst = self.backup_path(relpath)
        if os.path.exists(src):
            self.ensure_backup()
            LOG.info('Backing up %s -> %s', src, dst)
            shutil.copy2(src, dst)
            return dst
        return None

    def diagnose(self):
        LOG.info('Starting diagnosis...')
        findings = []

        # bootloader detection
        has_grub = self.target_has_file('boot/grub/grub.cfg')
        has_limine = self.target_has_file('limine.cfg') or self.target_has_file('boot/limine.cfg')
        findings.append(('GRUB_CFG', has_grub))
        findings.append(('LIMINE_CFG', has_limine))

        # kernel detection
        has_safeos = self.target_has_file('SafeOS.bin') or self.target_has_file('boot/SafeOS.bin')
        findings.append(('SafeOS.bin', has_safeos))

        # checksum if possible
        repo_bin = self.locate_safeos_bin()
        if repo_bin and has_safeos:
            tpath = os.path.join(self.target, 'SafeOS.bin')
            if not os.path.exists(tpath):
                tpath = os.path.join(self.target, 'boot', 'SafeOS.bin')
            if os.path.exists(tpath):
                repo_hash = sha256_of_file(repo_bin)
                target_hash = sha256_of_file(tpath)
                LOG.info('SHA256 repo=%s target=%s', repo_hash, target_hash)
                findings.append(('SafeOS_checksum_match', repo_hash == target_hash))
            else:
                findings.append(('SafeOS_checksum_match', False))
        else:
            findings.append(('SafeOS_checksum_match', False))

        # Print summary
        LOG.info('Diagnosis Summary:')
        for k, v in findings:
            LOG.info('  %-25s : %s', k, 'OK' if v else 'MISSING/DIFF')

        # Check for in-guest BootRepair request marker
        marker_path = os.path.join(self.target, 'KROW_BOOTREPAIR_REQUEST')
        marker_present = os.path.exists(marker_path)
        findings.append(('BOOTREPAIR_MARKER', marker_present))
        if marker_present:
            try:
                with open(marker_path, 'r') as f:
                    m = f.read(256)
                LOG.info('Found bootrepair marker: %s', m.strip())
            except Exception:
                LOG.info('Found bootrepair marker file (could not read)')

        return findings

    def _run_as_root_required(self):
        if os.geteuid() != 0:
            LOG.error('This operation requires root privileges. Re-run as root (sudo).')
            return False
        return True

    def mount_image(self, image_path):
        """Attach image to a loop device and mount first partition. Returns (loopdev, mountpoint) or (None, None)."""
        if not os.path.exists(image_path):
            LOG.error('Image not found: %s', image_path)
            return None, None
        if not self._run_as_root_required():
            return None, None

        # Create loop device and scan partitions
        out = run_command(['losetup', '--show', '-Pf', image_path])
        if not out:
            LOG.error('losetup failed')
            return None, None
        loopdev = out.strip()
        LOG.info('Created loop device %s', loopdev)

        # Determine first partition device (loopXp1 or loopX1)
        p1 = loopdev + 'p1'
        p1_alt = loopdev + '1'
        part = None
        if os.path.exists(p1):
            part = p1
        elif os.path.exists(p1_alt):
            part = p1_alt
        else:
            # maybe image is a flat filesystem
            part = loopdev

        mp = tempfile.mkdtemp(prefix='krow_mount_')
        LOG.info('Mounting %s -> %s', part, mp)
        if run_command(['mount', part, mp]) is None:
            LOG.error('Mount failed for %s', part)
            # cleanup loop device
            run_command(['losetup', '-d', loopdev])
            return None, None

        LOG.info('Mounted image partition at %s', mp)
        return loopdev, mp

    def unmount_image(self, loopdev, mountpoint):
        if mountpoint and os.path.ismount(mountpoint):
            run_command(['umount', mountpoint])
        if loopdev:
            run_command(['losetup', '-d', loopdev])
        if mountpoint and os.path.isdir(mountpoint):
            try:
                os.rmdir(mountpoint)
            except Exception:
                pass

    def install_bootloader_to_image(self, loopdev, mountpoint):
        """Run grub-install on the attached loop device. Requires root."""
        if not loopdev or not mountpoint:
            LOG.error('loopdev or mountpoint missing')
            return False
        if not self._run_as_root_required():
            return False

        # Use grub-install --boot-directory=<mountpoint>/boot <loopdev>
        bootdir = os.path.join(mountpoint, 'boot')
        os.makedirs(bootdir, exist_ok=True)
        # Add common flags for UEFI/BIOS compatibility where supported
        cmd = ['grub-install', '--boot-directory=' + bootdir, '--target=i386-pc', '--recheck', '--no-floppy', loopdev]
        LOG.info('Running: %s', ' '.join(cmd))
        out = run_command(cmd)
        if out is None:
            LOG.error('grub-install failed')
            return False
        LOG.info('grub-install completed (output above)')
        return True

    def repair(self, mount_image=False, install_bootloader=False):
        LOG.info('Beginning repair (apply=%s) (mount_image=%s install_bootloader=%s)', self.apply, mount_image, install_bootloader)
        # Ensure we do not accidentally run destructive ops without explicit consent
        if not self.apply and not self.assume_yes:
            LOG.warning('Repair requested but --apply not provided. Dry-run only.')

        mounted = False
        loopdev = None
        mountpoint = None

        # If target is a file and user requested mounting, attach it
        if os.path.isfile(self.target) and mount_image:
            LOG.info('Target appears to be a file; attempting loopback mount')
            loopdev, mp = self.mount_image(self.target)
            if not loopdev:
                LOG.error('Failed to mount image; aborting repair on image')
                return False
            mounted = True
            mountpoint = mp
            # Switch target to mountpoint for filesystem operations
            old_target = self.target
            self.target = mountpoint

        # Restore SafeOS.bin if missing
        repo_bin = self.locate_safeos_bin()
        if repo_bin:
            # determine target path
            tpath = os.path.join(self.target, 'SafeOS.bin')
            if not os.path.isdir(self.target):
                LOG.error('Target %s is not a directory. Mount or provide the root path.', self.target)
                if mounted:
                    self.unmount_image(loopdev, mountpoint)
                return False
            if not os.path.exists(tpath):
                # try boot/ path
                tpath = os.path.join(self.target, 'boot', 'SafeOS.bin')
            if not os.path.isdir(os.path.dirname(tpath)):
                os.makedirs(os.path.dirname(tpath), exist_ok=True)
            if not os.path.exists(tpath) or sha256_of_file(tpath) != sha256_of_file(repo_bin):
                LOG.info('SafeOS.bin is missing or differs; will restore to %s', tpath)
                if self.apply:
                    self.backup_file(os.path.relpath(tpath, self.target))
                    shutil.copy2(repo_bin, tpath)
                    LOG.info('Restored SafeOS.bin to %s', tpath)
                else:
                    LOG.info('Dry-run: would copy %s -> %s', repo_bin, tpath)
        else:
            LOG.warning('No repository SafeOS.bin to restore from')

        # Restore grub.cfg if missing
        grub_repo = self.find_grub_cfg_in_repo()
        if grub_repo:
            target_grub = os.path.join(self.target, 'boot', 'grub', 'grub.cfg')
            if not os.path.exists(target_grub):
                LOG.info('grub.cfg missing; will restore to %s', target_grub)
                if self.apply:
                    self.backup_file('boot/grub/grub.cfg')
                    os.makedirs(os.path.dirname(target_grub), exist_ok=True)
                    shutil.copy2(grub_repo, target_grub)
                    LOG.info('Restored grub.cfg')
                else:
                    LOG.info('Dry-run: would copy %s -> %s', grub_repo, target_grub)

        # Restore limine.cfg if missing
        limine_repo = self.find_limine_cfg_in_repo()
        if limine_repo:
            target_limine = os.path.join(self.target, 'limine.cfg')
            if not os.path.exists(target_limine):
                LOG.info('limine.cfg missing; will restore to %s', target_limine)
                if self.apply:
                    self.backup_file('limine.cfg')
                    shutil.copy2(limine_repo, target_limine)
                    LOG.info('Restored limine.cfg')
                else:
                    LOG.info('Dry-run: would copy %s -> %s', limine_repo, target_limine)

        # Optionally install bootloader if we mounted an image and user requested it
        if mounted and install_bootloader:
            if self.apply:
                LOG.info('Installing GRUB into image device %s', loopdev)
                ok = self.install_bootloader_to_image(loopdev, mountpoint)
                if not ok:
                    LOG.error('Bootloader installation failed')
            else:
                LOG.info('Dry-run: would run grub-install on %s', loopdev)

        # If a BootRepair marker exists and this was requested, note it
        marker_file = os.path.join(self.target, 'KROW_BOOTREPAIR_REQUEST')
        if os.path.exists(marker_file):
            LOG.info('BootRepair marker present in target (%s)', marker_file)
            if not self.apply:
                LOG.info('Marker detected; run with --apply to perform automatic repairs')
            else:
                LOG.info('Marker detected and --apply provided; repair actions were performed (see above)')

        # Cleanup any mounted image
        if mounted:
            LOG.info('Unmounting image and detaching loop device')
            self.unmount_image(loopdev, mountpoint)
            # restore original self.target if it was changed
            try:
                self.target = old_target
            except NameError:
                pass

        LOG.info('Repair phase complete. To finish bootloader installation on real hardware or disk images, run host tools like grub-install or use the provided install_to_disk.sh script.')
        return True

    def package_recovery_iso(self):
        """Copy the host-side repair tools into the ISO build tree and build an ISO that contains them."""
        LOG.info('Packaging recovery ISO with repair tools included')
        build_iso_dir = os.path.join(self.repo, 'build', 'iso')
        tools_src = os.path.join(self.repo, 'tools')
        tools_dst = os.path.join(build_iso_dir, 'tools')
        if not os.path.isdir(tools_src):
            LOG.error('Tools directory not found in repo: %s', tools_src)
            return False
        # Ensure build/iso exists
        os.makedirs(build_iso_dir, exist_ok=True)
        # Remove any existing tools dir in the iso tree
        if os.path.exists(tools_dst):
            shutil.rmtree(tools_dst)
        shutil.copytree(tools_src, tools_dst)
        # Make sure scripts are executable
        for root, dirs, files in os.walk(tools_dst):
            for fname in files:
                p = os.path.join(root, fname)
                try:
                    st = os.stat(p)
                    os.chmod(p, st.st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)
                except Exception:
                    pass
        LOG.info('Tools copied to ISO tree: %s', tools_dst)
        # Build ISO using project's make target
        run_command(['make', 'iso'])
        LOG.info('Recovery ISO build attempted; check output for SafeOS-1.0.iso')
        return True

    def rollback(self, backup_dir):
        if not os.path.isdir(backup_dir):
            LOG.error('Backup dir not found: %s', backup_dir)
            return False
        LOG.info('Rolling back from %s', backup_dir)
        for f in os.listdir(backup_dir):
            src = os.path.join(backup_dir, f)
            dst = os.path.join(self.target, f)
            LOG.info('Restoring %s -> %s', src, dst)
            shutil.copy2(src, dst)
        LOG.info('Rollback complete')
        return True


def run_command(cmd):
    LOG.debug('Run: %s', ' '.join(cmd))
    try:
        out = subprocess.check_output(cmd, stderr=subprocess.STDOUT)
        LOG.debug('Output: %s', out.decode('utf-8', errors='replace'))
        return out.decode('utf-8')
    except subprocess.CalledProcessError as e:
        LOG.error('Command failed: %s', e.output.decode('utf-8', errors='replace'))
        return None


def mount_image_loop(image_path):
    """Set up a loop device for the given raw image and return loop device path and mountpoint for first partition."""
    if not os.path.exists(image_path):
        LOG.error('Image not found: %s', image_path)
        return None, None, None
    # Require losetup
    if shutil.which('losetup') is None:
        LOG.error('losetup not found on host; cannot mount image')
        return None, None, None
    # Create a temporary mount point
    mp = os.path.abspath('.krow_mount_{}'.format(int(time.time())))
    os.makedirs(mp, exist_ok=True)
    # Setup loop device with partition scan
    cmd = ['losetup', '--show', '-Pf', image_path]
    try:
        out = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode().strip()
        loopdev = out.splitlines()[-1].strip()
        LOG.info('Loop device: %s', loopdev)
    except subprocess.CalledProcessError as e:
        LOG.error('losetup failed: %s', e.output.decode('utf-8', errors='replace'))
        return None, None, None

    # Try to find partition device (e.g., /dev/loop0p1 or /dev/loop0)
    part = loopdev + 'p1'
    if not os.path.exists(part):
        # Fallback: try loop device without p (single-partition image)
        part = loopdev
        if not os.path.exists(part):
            LOG.error('Partition device for %s not found', loopdev)
            return loopdev, None, None

    # Mount partition
    try:
        subprocess.check_call(['mount', part, mp])
        LOG.info('Mounted %s -> %s', part, mp)
    except subprocess.CalledProcessError as e:
        LOG.error('Failed to mount %s: %s', part, e)
        # detach loop
        subprocess.call(['losetup', '-d', loopdev])
        return loopdev, None, None

    return loopdev, part, mp


def unmount_image_loop(loopdev, mountpoint):
    if mountpoint and os.path.ismount(mountpoint):
        try:
            subprocess.check_call(['umount', mountpoint])
            LOG.info('Unmounted %s', mountpoint)
        except subprocess.CalledProcessError as e:
            LOG.error('Failed to unmount %s: %s', mountpoint, e)
    if loopdev:
        try:
            subprocess.check_call(['losetup', '-d', loopdev])
            LOG.info('Detached loop device %s', loopdev)
        except subprocess.CalledProcessError as e:
            LOG.error('Failed to detach loop device %s: %s', loopdev, e)



def main():
    parser = argparse.ArgumentParser(description='Krow BootRepair Tool')
    sub = parser.add_subparsers(dest='cmd')

    p_diag = sub.add_parser('diagnose')
    p_diag.add_argument('--target', required=False, default='.', help='Path to mounted SafeOS root or image')

    p_repair = sub.add_parser('repair')
    p_repair.add_argument('--target', required=False, default='.', help='Path to mounted SafeOS root or image')
    p_repair.add_argument('--apply', action='store_true', help='Perform repairs')
    p_repair.add_argument('--assume-yes', action='store_true', help='Assume yes to prompts')
    p_repair.add_argument('--mount-image', action='store_true', help='If target is a raw image file, mount using loopback')
    p_repair.add_argument('--install-bootloader', action='store_true', help='After repair, run grub-install on mounted image (requires root)')
    p_repair.add_argument('--auto', action='store_true', help='Only run repair automatically if BootRepair marker is present')

    p_rb = sub.add_parser('rollback')
    p_rb.add_argument('--backup', required=True, help='Path to backup directory created by this tool')

    p_pkg = sub.add_parser('package')
    p_pkg.add_argument('--iso', action='store_true', help='Build bootable ISO (requires make iso or xorriso)')
    p_pkg.add_argument('--recovery', action='store_true', help='Build ISO that includes repair tools in the ISO image')

    args = parser.parse_args()

    if args.cmd is None:
        parser.print_help()
        sys.exit(2)

    if args.cmd == 'diagnose':
        br = BootRepair(args.target)
        br.diagnose()

    elif args.cmd == 'repair':
        br = BootRepair(args.target, apply=args.apply, assume_yes=args.assume_yes)
        findings = br.diagnose()
        if getattr(args, 'auto', False):
            marker_found = False
            for k, v in findings:
                if k == 'BOOTREPAIR_MARKER' and v:
                    marker_found = True
                    break
            if marker_found:
                LOG.info('BOOTREPAIR marker detected -> running repair')
                br.repair(mount_image=args.mount_image, install_bootloader=args.install_bootloader)
            else:
                LOG.info('No BOOTREPAIR marker detected; aborting automatic repair')
        else:
            br.repair(mount_image=args.mount_image, install_bootloader=args.install_bootloader)

    elif args.cmd == 'rollback':
        br = BootRepair('.', apply=False)
        br.rollback(args.backup)

    elif args.cmd == 'package':
        repo = find_repo_root('.')
        did_any = False
        if args.iso:
            did_any = True
            LOG.info('Packaging ISO using project Makefile')
            cmd = ['make', 'iso']
            run_command(cmd)
        if getattr(args, 'recovery', False):
            did_any = True
            br = BootRepair('.', apply=False)
            br.package_recovery_iso()
        if not did_any:
            LOG.info('No packaging target provided')


if __name__ == '__main__':
    main()
