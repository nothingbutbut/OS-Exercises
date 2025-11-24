# 安装eBPF工具链
我在ubuntu 22.04上进行操作，运行以下命令安装eBPF工具链：
```bash
sudo apt-get update
sudo apt-get install -y bpfcc-tools libbpfcc-dev linux-headers-$(uname -r)
sudo apt install -y liblzma-dev clang-18 libclang-18-dev llvm-18-dev libclang-11-dev libelf-dev liblzma-dev libzstd-dev
sudo apt-get install libpolly-18-dev
sudo apt-get update
sudo apt-get install -y cmake
mkdir build && cd build
cmake ..
make
sudo make install
cmake .. -DPYTHON_CMD=python3
```
拉取bcc仓库
```bash
git clone git@github.com:iovisor/bcc.git
```

# 课堂上的例子
```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int LOOP = 10;

int main() {
    pid_t pid;
    int i;
    for(i=0; i<LOOP; i++){
        /* fork another process */
        pid = fork() ;
        if (pid < 0) { /*error occurred */
            fprintf(stderr, "Fork Failed");
            exit (-1) ;
        }
        else if (pid == 0) { /* child process */
            fprintf(stdout, "i=%d, pid=%d, parent pid=%d\n", i, getpid() , getppid());
        }
    }
    wait (NULL);
    exit (0) ;
}
```

```bash
cd ~/bcc/tools
gcc -O2 child_fork.c -o child_fork
sudo ./child_fork &
cd /root/bcc/tools
chmod +x wakeuptime.py
sudo "$(which python3)" /root/bcc/tools/wakeuptime.py -p "$(pgrep -n child_fork)"
```

# 尝试知乎blog上的安装方法
```bash
sudo apt purge bpfcc-tools libbpfcc python3-bpfcc
wget https://github.com/iovisor/bcc/releases/download/v0.25.0/bcc-src-with-submodule.tar.gz
tar xf bcc-src-with-submodule.tar.gz
cd ~/bcc
sudo apt install -y python-is-python3
sudo apt install -y bison build-essential cmake flex git libedit-dev zlib1g-dev libelf-dev libfl-dev python3-distutils
sudo apt install -y libllvm14 llvm-14-dev libclang-14-dev
mkdir build
cd build/
cmake -DCMAKE_INSTALL_PREFIX=/usr -DPYTHON_CMD=python3 ..   
make 
make install
```