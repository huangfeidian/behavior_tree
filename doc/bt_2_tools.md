# 行为树相关工具

## 编辑器

当前编辑器是用`qt5`编写的，分发的时候我把相关的`dll`也打包到一个`zip`里面，编译平台是`win 10, Qt 5.12`，不保证其他系统配置下可用。至于`linux`和`mac`下的二进制，我后面再尝试编译一下。

### 概览

![](images\editor_preview.png)

本编辑器支持多文件以`tab`形式同时编辑，编辑窗口分为了两个部分：

1. 树结构展示区 这里可以有多`tab`页，以图形的格式展示整个行为树的结构。每个节点的子节点都在当前节点的右侧，这个跟`unreal`那种上下布局不太一样，准备以后支持两种布局。每个节点上面的`label`分为了三个部分：
   1. 节点类型 `root sequence`等 一旦确定不可修改
   2. 节点编号 这个编号在一颗树内是唯一的非负整数 不可编辑 根节点永远保持为0
   3. 节点的展示信息 可以为此节点的自身信息或者`comment`信息所修改
2. 节点参数编辑界面
   1. `comment` 字段 是个文本编辑区域 默认为空 如果有值 对应的值会作为节点图形展示信息 不携带语义
   2. `color`字段 更改节点的颜色 不携带语义
   3. 其他的就是各个节点的具体参数编辑界面 如果是action节点 包括如下几个部分
      1. `action` 当前`action`节点的`action` 图里显示的`action`是`log` 下面的文本区域代表这个`log action`的意义是什么，后面的`return`文本编辑区域代表这个action的各个情况的返回值是什么
      2. `args` 这个是当前`action`的所有参数的编辑部分 当前有两个参数 分别是`log_level` 和`log_info`,他的可选值类型有两种 一个是`plain` 代表直接使用编辑器内输入的值，另外一种是`blackboard`，代表使用当前输入的`key`对应的黑板值，输入框里面允许输入的都是`json`字符串，但是目前不允许输入`object`类型的值。如果想输入`object`类型，请转换为`key value` 的`list`的`list`形式去输入。

### 文件的打开与保存

![](images\editor_file_edit.png)

如果文件被修改了，则标题栏显示的文件名后面会有*符号

### 节点的编辑

![](images\editor_node_edit.png)

节点编辑时，首先需要选中一个节点，然后按下对应的快捷键：

1. `Insert` 代表插入一个节点，作为当前节点排序最低的子节点
2. `Delete`代表删除一个节点， root节点不可删除
3. `MoveUp` 代表把提升当前节点在父节点里的排序
4. `MoveDown` 代表降低当前节点在父节点里的排序
5. `Copy` 代表把当前节点为根的子树复制
6. `Paste` 代表把上次复制的节点粘贴为当前节点排序最低的新的子节点
7. `Cut` 代表剪切当前节点

下图就是按下Insert的时候的截图， 这里有个搜索框，双击对应的行就会创建对应类型的节点：

![](images\editor_node_select.png)

另外如果树里面的节点太多，可以通过双击这个节点，将对应的子树进行折叠，同时这个节点上面会有一个十字交叉表示，再双击则会展开折叠：

![](images\editor_node_collapse.png)

### 节点参数编辑

节点的相关参数基本都是文本输入框，但是有两个例外：

1. 选择颜色按钮

![](images\editor_color_select.png)

2. `action`的搜索按钮

   由于`action`后面有很多，我们项目组已经有400多个，所以有搜索的需求，所以这里提供了`action`的搜索框，支持中英文搜索，双击对应行之后就会设置为当前`action`，同时对应的`args`编辑区域也会更新

![](images\editor_action_select.png)

### 导航

导航这里有两个按钮，一个是搜索，一个是跳转。

![](images\editor_navigate.png)

跳转这里会让你输入节点编号，然后就会使这个节点出现在图形区域中。

而导航，则会以搜索框的形式跳转：

![](images\editor_navigate_search.png)

## action parser

上面的编辑器依赖于我们提供一个`actions.json`的文件，里面提供了所有`agent`类的各个子类可以使用的`action`列表及对应的参数说明和`action`说明，返回值说明等等。下面是一个样例`action`的声明：

```c++
//! \fn	Meta(behavior_action) bool has_key(const std::string& bb_key);
//!
//! \brief	判断黑板内是否有特定key
//!
//! \author	Administrator
//! \date	2019/11/24
//!
//! \param	bb_key	key的名字
//! \return 如果有这个key返回true 否则返回false

Meta(behavior_action) bool has_key(const std::string& bb_key);
```

这里的注释采取的是`doxygen`的格式，对我们有用的信息包括：

1. `brief` 这个`action`的功能简介
2. `param` 所有的参数名称和对应的说明
3. `return` 这个`action`的返回值情况

每个函数都可以通过`Meta(behavior_action)`这个宏标记来注册：

```c++
#if defined(__meta_parse__)
#define Meta(...) __attribute__((annotate(#__VA_ARGS__)))
#else
#define Meta(...) 
#endif
```

这个宏在一般情况下不会有任何作用，只有在传入特定宏定义的时候才会注册为`attritbute`，这个`attribute`语法目前只有`clang`可以识别。

可导出的`action`函数的返回值只能为两类：

1. `bool` 代表这个函数是立即返回的，执行的是一个同步调用
2. `std::optional<bool>` 代表这个函数可能是异步返回的，会开启一个异步过程

而函数的参数类型目前只支持可以被`behavior_tree::common::any_value_type`容纳的类型，包括自身。

这些信息是通过基于`libclang`的工具抽取出来自动生成的，同样的还有`runtime`文件夹下所有的`generated_h`和`generated_cpp`文件，这些是自动生成的`c++`代码。这些都是依赖于`action_parser`，在`release1.1`里面我提供了最新的`action parser`，使用方式见`readme`，下面是我在`powershell`里面的一个使用样例。

> .\parser.exe ..\runtime\include\behavior_test.cpp -I ..\common\include -I ..\r
> untime\include\ -I $env:usr

这样执行之后就会在当前文件夹下生成新的`actions.json`，同时会更新`runtime\include\`下的所有自动生成的`c++`文件。

这个新的`actions.json`可以放到`data`目录下给编辑器去读取。

## 调试器

调试器这个暂时没空去完成了，因为会涉及到网络相关的内容，等我把后续的几篇关于行为树理论相关的写完看看有没有空。如果谁有兴趣想去先做的话，可以参考腾讯的`behaviac`:  [https://www.behaviac.com](https://www.behaviac.com/language/zh/首页/) 

