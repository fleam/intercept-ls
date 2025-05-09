
## 👉 指定目录 ls 命令拦截

```bash
$ gcc -fPIC -shared -o intercept_ls.so intercept_ls.c -ldl
```
## 👉 测试

```bash
$ LD_PRELOAD=/home/fleam/intercept-ls/intercept_ls.so ls /home/fleam/intercept-ls/test
$ export LD_PRELOAD=/home/fleam/intercept-ls/intercept_ls.so
```

## 👉 永久写入 shell

```bash
echo 'export LD_PRELOAD=/home/fleam/intercept-ls/intercept_ls.so' >> ~/.bashrc
source ~/.bashrc
```


## ✅ 简单一句话总结

我们利用了 **动态链接器的 `LD_PRELOAD` 机制**，在程序运行前**替换掉某些标准库函数的实现**，从而实现对程序行为的控制。

---

## 🧠 原理详解

### 1. `LD_PRELOAD` 是什么？

- `LD_PRELOAD` 是 Linux 动态链接器（`ld.so`）的一个环境变量。
- 它允许你在程序启动前，**优先加载指定的共享库（`.so` 文件）**。
- 如果这个 `.so` 中定义了与标准库中同名的函数（如 `opendir`），它会**覆盖标准库中的实现**。

---

### 2. 函数 Hook（钩子）是什么？

- 我们自己实现了 `opendir()`、`readdir()` 等函数。
- 当 `ls` 命令调用这些函数时，**先执行我们的代码**。
- 在我们的代码里判断：
  - 当前进程是不是 `ls`
  - 要打开的路径是不是 `/home/fleam/intercept-ls/test`
- 如果是，则返回 `NULL` 阻止访问；否则调用原始的标准库函数。

这就是所谓的 **函数 Hook / 拦截 / 替换**。

---

### 3. 为什么会影响其他命令？

- 因为所有使用 `libc` 的程序都会调用 `opendir()`。
- 如果你不加判断直接拦截，那 `git add .`、`find`、`rm -r` 等命令也会受到影响。

所以我们加上了如下判断：

```c
if (strstr(proc_self, "/ls") == NULL) {
    return real_opendir(name); // 不是 ls 就放行
}
```

这样只有 `ls` 命令才会被拦截。

---

## 🛠️ 技术栈组成

| 技术 | 说明 |
|------|------|
| `LD_PRELOAD` | 动态链接器特性，用于优先加载我们自己的 `.so` |
| `dlsym(RTLD_NEXT, "func")` | 获取下一个（即系统原本的）函数地址 |
| `opendir`, `readdir` | 被 hook 的目标函数，用于控制目录访问 |
| `readlink("/proc/self/exe")` | 获取当前进程的可执行文件路径 |
| `strstr(..., "/ls")` | 判断是否是 `ls` 命令 |

---

## 🔍 实际流程图

```
ls 命令执行
   ↓
调用 opendir()
   ↓
你的 intercept_ls.so 中的 opendir() 被调用
   ↓
判断是否是 ls 进程 + 是否访问指定路径
   ├─ 是 → 打印提示并返回 NULL（阻止访问）
   └─ 否 → 调用真实 opendir()（dlsym 得到）
```

---

## ✅ 总结

| 技术点 | 原理 | 应用场景 |
|--------|------|----------|
| `LD_PRELOAD` | 强制优先加载自定义 `.so` | 修改或增强程序行为 |
| `dlsym(RTLD_NEXT)` | 获取原始函数指针 | 实现函数 hook |
| `opendir` 拦截 | 控制目录访问权限 | 隐藏目录内容、限制访问 |
| 进程名判断 | 只影响特定命令 | 防止误伤 `git`、`find` 等命令 |

---

如果你想进一步扩展功能，比如：

- 支持配置多个要拦截的路径
- 支持拦截特定命令（不只是 `ls`）
- 支持隐藏特定文件名
