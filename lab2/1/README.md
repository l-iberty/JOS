# Lab2-1

## 使用 vscode-gdb 调试代码
1. 在目标机上安装插件“Native Debug”
2. "Run -> Add Configuration... -> GDB", 编辑`launch.json`:
```json
{
	// Use IntelliSense to learn about possible attributes.
	// Hover to view descriptions of existing attributes.
	// For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
	"version": "0.2.0",
	"configurations": [
		{
			"name": "Debug",
			"type": "gdb",
			"request": "attach",
			"remote": true,
			"target": "localhost:1234",
			"cwd": "${workspaceRoot}",
			"executable": "${workspaceRoot}/lab2/1/obj/kernel/kernel",
			"valuesFormatting": "parseText"
		}
	]
}
```
3. `make qemu-gdb`准备好后，在 vscode 里`F5`便可以开启调试并命中断点：
![](imgs/vscode-gdb.png)

但是由于未知原因，有的变量无法在`VARIABLES`窗口查看，还是要在`DEBUG CONSOLE`里使用gdb命令进行操作。