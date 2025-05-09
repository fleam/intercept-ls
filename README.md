
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
