# OpenWrt 图形与视频 feed

本仓库是 [hnt510-great/openwrt](https://github.com/hnt510-great/openwrt) 定制主树使用的私有源码 feed。
项目面向 x86_64/glibc、APK、原生 systemd 和 GNOME 桌面；软件选择以主树
保存的完整配置为准。

| 项目 | 值 |
| --- | --- |
| Feed 名称 | `video` |
| 本仓库分支 | `master` |
| 原始上游 | [https://github.com/openwrt/video](https://github.com/openwrt/video)（`master`） |

本 feed 基于 OpenWrt video，提供图形库、多媒体依赖、显示组件及应用的
构建配方，并保存本项目的桌面和虚拟化集成历史。

## 本项目的主要集成

- **图形与虚拟化**：Mesa 的 virtualization 变体、GLVND/EGL/OpenGL 分发、
  VirGL 所需依赖，以及使用交叉构建指定 Python 解释器的构建修复。
- **运行时元数据**：GdkPixbuf、Graphene、HarfBuzz、Pango 等配方的 GIR/
  typelib 和构建依赖声明，供主树 GTK、Python GI、GNOME 与虚拟机控制台使用。
- **桌面应用依赖**：Wayland、字体布局、图像解码及图形相关组件；Firefox ESR
  的预编译包配方位于主树 `package/custom`，并使用这些运行时库。
- **历史 Weston 集成**：仓库保留早期桌面、镜像输入、RDP/音频和终端等改动。
  当前完整桌面配置已切换到 GNOME，不能把保留的 Weston 配方视作默认桌面。

GNOME Shell、Mutter、桌面会话与远程访问的主要自定义配方位于主树
`package/custom`。QEMU/SPICE 的另一部分修改位于 `packages` feed，
libvirt/virt-manager 也依赖主树，因此应使用配套源码和配置构建。

虚拟 GPU 的 VirGL 验证、物理 GPU 驱动、视频解码和远程串流分别有不同的
验收范围。较早文档中的“无 X11/GLX”描述属于当时配置；构建选项应以当前
Mesa 配方和主树配置为准。

## 获取与构建

先在构建机登录有权访问全部私有仓库的 GitHub 账号，再获取完整主树：

```sh
gh auth login --hostname github.com --git-protocol https --web
gh auth setup-git
git clone https://github.com/hnt510-great/openwrt.git
cd openwrt
./scripts/feeds update -a
./scripts/feeds install -a
```

主树 `feeds.conf.default` 已包含：

```text
src-git video https://github.com/hnt510-great/openwrt-video.git;master
```

在现有主树中只更新本 feed，可执行：

```sh
./scripts/feeds update video
./scripts/feeds install -a -p video
```

先在主树配置中选择所需包并运行 `make defconfig`。单包构建示例：

```sh
make package/feeds/video/mesa/compile V=s
```

以上命令均从主树根目录运行。保留已有 `.config`；新 checkout 的配置恢复
步骤见 [本地构建指南](https://github.com/hnt510-great/openwrt/blob/main/docs/local-build.md)。`install` 安装的是构建配方链接，
不会在设备上安装软件。完整 `update -a` 还会检查并可能修改主树的 custom 配方。

## 维护与验证

`origin` 指向本私有仓库。新 clone 需要合并原始上游时，先添加一次 `upstream`：

```sh
git -C feeds/video remote add upstream https://github.com/openwrt/video.git
git -C feeds/video fetch upstream
git -C feeds/video merge upstream/master
# 审阅、解决冲突并完成相关验证后推送
git -C feeds/video push origin master
```

已有 `upstream` 时跳过添加命令。`scripts/feeds update` 拉取本私有分支，
不会自动合并原始上游。本地集成修改与主树的提交分别管理，保留现有提交、
stash 和未提交文件。主树的 `patches/feeds/` 和功能清单是历史备份，
不要向已经包含修改的 feed 重复应用补丁。

涉及已发布 APK 内容的修改必须产生新包版本，不能覆盖同一包身份。源码
检查、包构建、VM 与物理设备验证应分别记录；本文汇总已有设计和验证记录，
不新增构建或设备验证结论。私有源码托管说明见
[GitHub 源码托管](https://github.com/hnt510-great/openwrt/blob/main/docs/github-source-hosting.md)。

## 设计与验证文档

- [当前 GNOME 桌面](https://github.com/hnt510-great/openwrt/blob/main/docs/gnome-desktop.md)
- [GNOME 验证范围](https://github.com/hnt510-great/openwrt/blob/main/docs/gnome-desktop-validation.md)
- [VirtIO/SPICE 集成与历史配置](https://github.com/hnt510-great/openwrt/blob/main/docs/virt-manager.md)
- [SPICE/VirGL 验证](https://github.com/hnt510-great/openwrt/blob/main/docs/virtio-spice-validation.md)
- [Firefox 与 VA-API](https://github.com/hnt510-great/openwrt/blob/main/docs/firefox.md)
- [桌面内核支持](https://github.com/hnt510-great/openwrt/blob/main/docs/desktop-kernel.md)

## 许可与上游

保留上游版权和许可。各软件包源码的许可按其配方 `PKG_LICENSE` 及源码声明执行。
