# behavior_tree
一组关于行为树的相关工具：包括运行时，编辑器， 调试器，和元数据工具， 采用`C++17 `和`Qt5`编写，关于更多的内容介绍，参见`doc`目录.

## 依赖

本项目由cmake来组织依赖，所有子项目共享的依赖有：

1. C++17 
2. json https://github.com/nlohmann/json
3. spdlog https://github.com/gabime/spdlog
4. magic_enum https://github.com/Neargye/magic_enum
5. any_container https://github.com/huangfeidian/any_container

## 运行时

运行时相关代码在`runtime`目录下，会生成后一个行为树的`lib`，里面有一个`example`来运行样例行为树文件。



## 编辑器
编辑器代码在`editor`目录下，相关代码已经被单独剥离到一个新项目` https://github.com/huangfeidian/tree_editor `，当前只需要继承一下即可，关于编辑器的使用参考`tree_editor`项目里的`editor`项目。

## 调试器

调试器代码在`debugger`目录下，相关代码已经被单独剥离到一个新项目` https://github.com/huangfeidian/tree_editor `，当前只需要继承一下即可，关于调试器的使用参考`tree_editor`项目里的`debugger`项目。



