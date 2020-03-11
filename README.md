# behavior_tree
一组关于行为树的相关工具：包括运行时，编辑器， 调试器，和元数据工具， 采用`C++17 `和`Qt5`编写，关于更多的内容介绍，参见`doc`目录.

## 依赖

本项目由cmake来组织依赖，所有子项目共享的依赖有：

1. C++17 
2. json https://github.com/nlohmann/json
3. spdlog https://github.com/gabime/spdlog
4. magic_enum https://github.com/Neargye/magic_enum

## 运行时

运行时相关代码在`runtime`目录下，会生成后一个行为树的`lib`，里面有一个`example`来运行样例行为树文件。

## 元数据工具

元数据工具相关代码在`action_parser`目录下，用来预处理运行时内的`c++`文件，来自动生成胶水代码和导出所有行为树的可用接口，这个行为树的可用接口可以被编辑器和调试器使用。这个工具的代码其实是我的另外一个项目生成的，参见 `https://github.com/huangfeidian/meta`  这个`repo`里`example` 目录下的`generate_behavior`工程，把这个工程生成的`exe`放在当前目录下，重命名为`parser.exe`, 执行一下当前目录下的`run_parser.bat`，就会补充所有的胶水代码和导出所有可用的action列表到`actions.json`，这个文件最后要转移到顶层项目的`data`目录下

## 编辑器
编辑器代码在`editor`目录下，相关代码已经被单独剥离到一个新项目` https://github.com/huangfeidian/tree_editor `，当前只需要继承一下即可，关于编辑器的使用参考`tree_editor`项目里的`editor`项目。

## 调试器

调试器代码在`debugger`目录下，相关代码已经被单独剥离到一个新项目` https://github.com/huangfeidian/tree_editor `，当前只需要继承一下即可，关于调试器的使用参考`tree_editor`项目里的`debugger`项目。



