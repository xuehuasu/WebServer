include ./build/Makefile

# 目标文件所在目录
OBJ_DIR = ../objs
# 模块名
modules = buffer log pool timer http server
# 模块所在目录
Modules = $(addprefix ./,$(modules))
# 可执行文件所在目录
BIN := ../bin


all: ${BIN}/server


obj = $(wildcard ${OBJ_DIR}/*.o)

${BIN}/server: child main.cpp
	$(CC) $(CFLAGS) ${OBJ_DIR}/*.o main.cpp -o $@ -lmysqlclient

child: ${obj}
	@for module in $(Modules); do \
        $(MAKE) -C $$module; \
    done

clean:
	rm -f ${OBJ_DIR}/main.o
	@for module in $(Modules); do \
        $(MAKE) -C $$module clean; \
    done