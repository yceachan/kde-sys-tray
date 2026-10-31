---
title: 上游基线
tags: [KDE, Plasma]
note_types: document
created: 2026-10-30
updated: 2026-10-31

---

# 上游基线

本项目从 KDE Plasma 官方系统托盘派生，以独立插件标识交付。上游源码的版权与逐文件许可证声明保留在对应文件中。

| 项目 | 固定值 |
| --- | --- |
| 仓库 | https://invent.kde.org/plasma/plasma-workspace |
| 下载镜像 | https://github.com/KDE/plasma-workspace |
| 标签 | `v6.7.5` |
| 提交 | `cc1ab7963f94a9b76836412855caed6b94066188` |
| 原插件 | `org.kde.plasma.systemtray` |
| 派生插件 | `local.plasma.windowstray` |

## 源码映射

| 上游路径 | 本项目路径 |
| --- | --- |
| `applets/systemtray/` | `src/` |
| `statusnotifierwatcher/systemtraytypedefs.h` | `src/systemtraytypedefs.h` |
| `libdbusmenuqt/` | `libdbusmenuqt/` |
| `libkmpris/dbus/org.freedesktop.DBus.Properties.xml` | `dbus/org.freedesktop.DBus.Properties.xml` |

顶层 CMake 将托盘及其菜单依赖提取为独立构建；保留的上游 `src/tests/` 和 `libdbusmenuqt/test/` 辅助程序不参与默认构建。派生组件复用系统安装的 KDE QML 模块，不安装或覆盖官方的私有 `systemtrayplugin`。

派生改动集中于应用拖拽、显隐配置与应用排序，并更改插件名称和元数据。原始元数据中的多语言“系统托盘”名称改为本项目中英文名称，避免组件选择器中混淆。

## 许可证

整体派生组件采用 **GPL-2.0-or-later**，GPL v2 正文见 [LICENSE](LICENSE)；各文件声明中的 `or-later` 允许选择 GPL v2 或后续版本。QML、菜单依赖及测试保留各自的 SPDX 声明，所需许可证正文与 KDE 接受许可证说明保存在 [LICENSES/](LICENSES/)。

上游原作者的版权声明保留在源码中，派生修改的版权为 `2026 yceachan`。修改过的上游文件标注修改日期与用途；从上游面板布局提取的 `PanelTrayGrid.qml` 同样保留原作者署名。继续分发或派生时，应保留版权及许可证声明，并按对应条款提供源码。
