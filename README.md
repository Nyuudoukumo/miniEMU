# miniEMU

这是一生一芯 v24.07 E4 阶段的 miniEMU 实现

## 目录结构

目录结构基于[lcthw](https://docs.kilvn.com/lcthw-zh/)的c-skeleton.  
`src`: 存放miniEMU的主要逻辑代码和debug工具代码.  
`tests`: 存放测试代码.  

## 项目运行

首先根据[ysyx-E3](https://ysyx.oscc.cc/docs/2407/e/3.html)配置系统环节，然后获取测试所需的文件:  

```bash
wget https://ysyx.oscc.cc/slides/resources/archive/logisim-bin.tar.bz2
```

然后解压:  

```bash
tar -xvf logisim-bin.tar.bz2 && rm logisim-bin.tar.bz2
```

之后将测试所需的文件从hex转为bin文件:  

```bash
cd logisim-bin
tail -n +2 sum.hex | sed -e 's/.*: //' -e 's/ /\n/g' | sed -e 's/\(..\)\(..\)\(..\)\(..\)/\4 \3 \2 \1/' | xxd -r -p > sum.bin
tail -n +2 mem.hex | sed -e 's/.*: //' -e 's/ /\n/g' | sed -e 's/\(..\)\(..\)\(..\)\(..\)/\4 \3 \2 \1/' | xxd -r -p > mem.bin
tail -n +2 vga.hex | sed -e 's/.*: //' -e 's/ /\n/g' | sed -e 's/\(..\)\(..\)\(..\)\(..\)/\4 \3 \2 \1/' | xxd -r -p > vga.bin
```

获取`valgrind`（如果不打算安装`valgrind`的话则在`tests/runtests.sh`中去除`$VALGRIND `）:

```bash
sudo apt install valgrind
```

在项目根目录中执行`make`将运行三个测试，即`test_addi_jalr`, `test_sum`和`test_mem`，测试日志存放于`tests\tests.log`.  
在`src`目录执行`make ARCH=native run`会测试`vga`，将会在一个弹出窗口中渲染`一生一芯`的logo.

