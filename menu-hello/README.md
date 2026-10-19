# MenuOS hello 命令实验

学号：20262811

基于 [mengning/menu](https://github.com/mengning/menu) 增加 `hello` 菜单命令，输入后输出：

```text
hello 20262811
```

命令处理函数及注册代码位于 [test.c](test.c)。已执行 `make rootfs` 重新制作文件系统，并在 WSL Ubuntu 中使用 Linux 5.15 内核和 QEMU 启动测试；`help` 正常列出 `hello`，执行结果符合预期。

- [QEMU 测试截图](artifacts/hello-20262811.png)
- [完整串口测试日志](artifacts/qemu-serial.log)

测试使用 Linux 5.15；课程原始环境为 Linux 3.18.6。

原项目来自 [mengning/menu](https://github.com/mengning/menu)，保留其 Apache License 2.0 许可证及原有版权声明。
