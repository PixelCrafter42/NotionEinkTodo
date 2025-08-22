### 简介

我喜欢 Notion，也钟爱墨水屏。我一直想要一台能实时显示待办事项的设备，帮助我更专注地完成重要任务。

M5Stack Paper S3 帮我把这个想法变成了现实。一个月前，我还没有任何相关实践经验。得益于 M5Stack 的 UIFlow，我很快搭建了原型；在 AI 的辅助下完善了代码，并用 Arduino 重构出一个响应更快的版本。整个过程既有趣又充满成就感。

这个项目基于 M5Stack Paper S3——一块集成 ESP32 与触控墨水屏的精致开发板。我利用其联网与触控能力，构建了一个能与 Notion 同步待办数据的看板：在设备上轻触即可勾选完成，状态会实时同步回 Notion 的 Todo List。我还使用 SD 卡存放字体与配置，并为看板加入了配置服务器模式，直接通过网页修改配置，无需频繁插拔 SD 卡。

### 一些界面截图
![cover](https://i.imgur.com/FkcCQG8.jpeg)
![list](https://i.imgur.com/HfFJZxJ.jpeg)
![config](https://i.imgur.com/rMsshyo.jpeg)
![server]()
### 使用教程

1. 安装 Arduino 开发环境与所需库，参考[官方教程](https://docs.m5stack.com/en/arduino/m5papers3/program)。
2. 将仓库中的以下文件拷贝到 SD 卡根目录：
    - cover.png（启动封面）
    - CNFont.vlw（必需，用于设置中文字体；可用[字体创建工具](https://vlw-font-creator.m5stack.com/)自定义）
    - config.json（必需，填写 Wi‑Fi、Notion API 与 Notion 数据库相关配置）
3. 使用 Arduino 打开 .ino 文件，编译并将固件上传到 Paper S3。
4. 开机体验！

### 参考

- https://docs.m5stack.com/en/arduino/m5papers3/program
- https://developers.notion.com/