# OpenWrt graphics and video feed

[简体中文](README.md) | **English**

This private source feed is used by the customized
[hnt510-great/openwrt](https://github.com/hnt510-great/openwrt) main tree.
The project targets x86_64/glibc, APK, native systemd and GNOME. Package
selection follows the main tree's saved configuration.

| Item | Value |
| --- | --- |
| Feed name | `video` |
| Repository branch | `master` |
| Original upstream | [https://github.com/openwrt/video](https://github.com/openwrt/video) (`master`) |

Based on OpenWrt video, this feed contains graphics libraries, multimedia
dependencies, display components and applications, while preserving local
desktop and virtualization integration history.

## Main integrations

- **Graphics and virtualization:** the Mesa virtualization variant,
  GLVND/EGL/OpenGL dispatch, VirGL dependencies and fixes for using the
  Python interpreter selected by the cross-build configuration.
- **Runtime metadata:** GIR/typelib support and build dependencies in
  GdkPixbuf, Graphene, HarfBuzz, Pango and related recipes for GTK, Python GI,
  GNOME and virtual-machine consoles.
- **Desktop application dependencies:** Wayland, font layout, image decoding
  and graphics components. The prebuilt Firefox ESR recipe lives in the
  main tree's `package/custom` and uses these runtime libraries.

The main GNOME Shell, Mutter, session and remote-access recipes live in
`package/custom`. Other QEMU/SPICE changes live in `packages`; libvirt and
virt-manager also depend on the main tree. Build from matching source and
configuration across those repositories.

Virtual-GPU VirGL, physical GPU drivers, video decoding and remote streaming
have separate acceptance scopes. Graphics build options follow the current
Mesa recipe and main-tree configuration.

## Get the source and build

Authenticate on the build machine with an account that can read every private
repository, then obtain the complete main tree:

```sh
gh auth login --hostname github.com --git-protocol https --web
gh auth setup-git
git clone https://github.com/hnt510-great/openwrt.git
cd openwrt
./scripts/feeds update -a
./scripts/feeds install -a
```

The main tree's `feeds.conf.default` already contains:

```text
src-git video https://github.com/hnt510-great/openwrt-video.git;master
```

To update only this feed in an existing main tree:

```sh
./scripts/feeds update video
./scripts/feeds install -a -p video
```

Select the desired package in the main configuration and run `make defconfig`.
Example individual package build:

```sh
make package/feeds/video/mesa/compile V=s
```

Run these commands from the main-tree root. Preserve an existing `.config`;
see [local build](https://github.com/hnt510-great/openwrt/blob/main/docs/local-build.md) for setup in a new checkout.
`install` creates build-recipe links rather than installing software on a
device. The full `update -a` also checks and can modify main-tree custom recipes.

## Maintenance and validation

`origin` points to this private repository. To merge original upstream changes
in a new clone, add `upstream` once:

```sh
git -C feeds/video remote add upstream https://github.com/openwrt/video.git
git -C feeds/video fetch upstream
git -C feeds/video merge upstream/master
# Review, resolve conflicts and validate before pushing
git -C feeds/video push origin master
```

Skip the add command if `upstream` already exists. `scripts/feeds update` pulls
the private branch; it does not merge original upstream automatically. Feed
integration and main-tree commits are separate. Preserve existing commits,
stashes and uncommitted work. Main-tree `patches/feeds/` files and feature
manifests are historical backups; do not reapply changes already present.

Changes to published APK contents require a new package version, preserving
immutable package identities. Record source checks, package builds, VM tests
and physical-device validation separately. This page summarizes existing
design and evidence without claiming new build or device results. See
[private GitHub hosting](https://github.com/hnt510-great/openwrt/blob/main/docs/github-source-hosting.md).

## Design and validation documents

- [Current GNOME desktop](https://github.com/hnt510-great/openwrt/blob/main/docs/gnome-desktop.md)
- [GNOME validation scope](https://github.com/hnt510-great/openwrt/blob/main/docs/gnome-desktop-validation.md)
- [VirtIO/SPICE integration](https://github.com/hnt510-great/openwrt/blob/main/docs/virt-manager.md)
- [SPICE/VirGL validation](https://github.com/hnt510-great/openwrt/blob/main/docs/virtio-spice-validation.md)
- [Firefox and VA-API](https://github.com/hnt510-great/openwrt/blob/main/docs/firefox.md)
- [Desktop kernel support](https://github.com/hnt510-great/openwrt/blob/main/docs/desktop-kernel.md)

## License and upstream

Retain upstream copyrights and licenses. Each package's source follows its
recipe's `PKG_LICENSE` and source declarations.
