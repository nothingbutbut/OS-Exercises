我配置的是uCore环境，我将原仓库进行了fork，新的仓库已经设置为private以防止抄袭。
```bash
git clone git@github.com:nothingbutbut/uCore-Tutorial-Code.git
```

我没有按照教程说明安装在/usr/local下，而是直接装在/root目录，运行的指令如下：
```bash
sudo wget https://static.dev.sifive.com/dev-tools/freedom-tools/v2020.08/riscv64-unknown-elf-gcc-10.1.0-2020.08.2-x86_64-linux-ubuntu14.tar.gz
sudo tar xzf riscv64-unknown-elf-gcc-10.1.0-2020.08.2-x86_64-linux-ubuntu14.tar.gz
sudo mv riscv64-unknown-elf-gcc-10.1.0-2020.08.2-x86_64-linux-ubuntu14 riscv64-unknown-elf-gcc

export PATH="/root/riscv64-unknown-elf-gcc/bin:$PATH"

# 如果链接失效，可以用官网链接下载并提醒助教更新云盘地址： https://more.musl.cc/10/x86_64-linux-musl/riscv64-linux-musl-cross.tgz
# 注意这里是 10.2.1 版本，如果从其他地方下载请务必注意版本号一致
sudo wget -O riscv64-linux-musl-cross.tgz "https://cloud.tsinghua.edu.cn/f/fb3c598e7e214a828e6b/?dl=1"

sudo tar xzf riscv64-linux-musl-cross.tgz

sudo apt update
sudo apt install cmake

# 安装编译所需的依赖包
sudo apt install autoconf automake autotools-dev curl libmpc-dev libmpfr-dev libgmp-dev \
              gawk build-essential bison flex texinfo gperf libtool patchutils bc ninja-build \
              zlib1g-dev libexpat-dev pkg-config  libglib2.0-dev libpixman-1-dev git tmux python3
# 下载源码包
# 如果链接失效，可以使用官网链接下载并提醒助教更新云盘地址： https://download.qemu.org/qemu-7.0.0.tar.xz
wget -O qemu-7.0.0.tar.xz "https://cloud.tsinghua.edu.cn/f/8ba524dbede24ce79d06/?dl=1"
# 解压
tar xJf qemu-7.0.0.tar.xz
# 编译安装并配置 RISC-V 支持
cd qemu-7.0.0
./configure --target-list=riscv64-softmmu,riscv64-linux-user
make -j$(nproc)

# 检查
qemu-system-riscv64 --version
qemu-riscv64 --version
```
发现qemu安装成功

试运行
```bash
git checkout ch1
make run LOG=debug
```
成功运行ch1的hello world了

过程中没有遇到问题