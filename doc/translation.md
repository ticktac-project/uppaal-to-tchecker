# Translation of Uppaal models into TChecker format

`utot` is not able to compile all [Uppaal] models. In this document we give some details on supported and unsupported features of [Uppaal] input format but we do not detail all the translation algorithm.

In the following we assume that both specification languages used by [Uppaal][utap] and [TChecker][tcksyntax] are familiar to the reader.

## Table of contents

* [Basic types](#basic-types)
* [Arrays](#arrays)
* [Processes](#processes)
* [Templates](#templates)
* [Synchronization](#synchronizations-of-channels)
* [Selections](#selections)
* [Unsupported features](#unsupported-features)

## Basic types

Except clocks, which remain unchanged, basic types of [Uppaal] are translated as integer variables in [TChecker]. For instance, following variables:
```
int n; 
int [-1,2] m; 
const int [-1,2] M = 2; 
bool is_true = true; 
scalar[2] S;
clock C; 
```

are translated as:
```
# int:array-size:min-value:max-value:initial-value:identifier 
int:1:-32768:32767:0:n
int:1:-1:2:0:m 
int:1:2:2:2:M 
int:1:0:1:1:is_true
int:1:0:1:0:S
    
# clock:array-size:identifier 
clock:1:C
```

## Arrays

Only arrays supported by [TChecker] are translated as is, that is to say, one-dimension arrays indexed from 0 and with the cells initialized with the same value. In other cases, `utot` flatten arrays and split them in individual variables. For instance, the following two-dimension array of scalar values
```
typedef int [-1,4] sometype t;
sometype t tab2[2][3] = { { -1, 0, 1 }, { 2, 3, 4 } };
```

is translated as 6 variables (one per cell):
```
int:1:-1:4:-1:tab2_0_0
int:1:-1:4:0:tab2_0_1
int:1:-1:4:1:tab2_0_2
int:1:-1:4:2:tab2_1_0
int:1:-1:4:3:tab2_1_1
int:1:-1:4:4:tab2_1_2
```
In the next example, three 1-dimension arrays are declared. 
```
typedef int [-1,2] sometype t; 
sometype t tab1[2];
sometype t tab2[2] = { 1, 1 }; 
sometype t tab3[2] = { 0, 1 };
```
The third one is initialized with different values for cells; so it is split into separate variables:
```
int:2:-1:2:0:tab1
int:2:-1:2:1:tab2
int:1:-1:2:0:tab3_0
int:1:-1:2:1:tab3_1
```

In the case of arrays with shifted indices, as for instance the array `values` below:
```
typedef int [999, 1005] pid t;
int [0,3] values[pid t];
pid t i;
process P () {
    state s0, s1 { values[i] < 2 };
    init s0;
}
```

the offset of cells is explicitly computed in each expression where the  array cells occur (here in the invariant of location `s1`):
```
int:7:0:3:0:values
int:1:999:1005:999:i
process:P
location:P:s0{initial:}
location:P:s1{invariant:(1 && (values[i-999] < 2))}
```

## Processes

There is no magic to translate Uppaal automata into their TChecker version:
* _locations_ are assigned to their associated process. Attributes are translated straightforwardly using their corresponding in TChecker. Expressions are translated wrt to the expressive power of TChecker syntax.  
* Similarly to locations, _edges_ are translated easily. The main difference with Uppaal is the enumeration of `select`statements (see [Selections](#selections)). 

## Templates

[Uppaal] allows the declaration of class of timed automata called _templates_. These templates might be parameterized with constants, variables and even reference to variables. 

In [Tchecker] input format, all instances of templates must be declared explicitly. In order to enumerate all instances of parameters, `utot` tries to substitute parameters with their actual value. If during the instantiation process an expected constant can not be evaluated then an error is generated. 

`utot` supports partial instantiation of templates which means that it enumerates all possible values of parameter with unspecified value. Each instance is assigned an identifier built from the template identifier follow by couples _param_\__value_ for each parameter _param_ and each possible value for _param_.

For instance, the following model
```
typedef int [1,2] pid t;
typedef scalar[3] key t;
process Process(const pid t pid, key t k) { }
system Process;
```

is translated into 6 instances of the process `Process` as :
```
process:Process_k_0_pid_1
process:Process_k_0_pid_2
process:Process_k_1_pid_1
process:Process_k_1_pid_2
process:Process_k_2_pid_1
process:Process_k_2_pid_2
```
 
For each instance of a template its local elements are duplicated. Variables and clocks are prefixed with the identifier of the instance:
```
process Q() { bool x; state s0; init s0; }
P1 := Q();
P2 := Q();
system P1, P2;
```

should yield the following [TChecker] model:
```
process:P1
int:1:0:1:0:P1_x
location:P1:s0{initial:}
process:P2
int:1:0:1:0:P2_x
location:P2:s0{initial:} 
```

Finally, parameters are replaced by their values. In the following example,
``` 
process Proc (clock &c, const int B) { 
    state s0; 
    init s0;
    trans s0 -> s0 { guard c < B; };
}
clock x[2];
S = Proc(x[1], 7);
system S;
```

the reference to a clock `c` and the integer constant `B` are replaced in the instance by their respective values, `x[1]` and `7`:
```
clock:2:x
process:S
location:S:s0{initial:}
event:tau edge:S:s0:s0:tau{provided:(x[1] < 7)}
```
    
## Synchronizations of channels

[Uppaal] permits to synchronize process with channels. Three synchronization mechanisms are available: CCS-like synchronization, broadcast and CSP-like synchronization. The latter is not supported (actually we did not found examples with this kind of synchronization).

### CCS-like synchronization

In this mode, an _emitter_ and a _receiver_ are synchronized on a given channel; say _a_ for instance. Syntactically, the edges that emit on _a_ are labelled with ` sync a!` and edges that receive on _a_ are labelled with `sync a?`. The synchronization is _strong_: an emitter needs a receiver to emit and a receiver needs an emitter to receive. In order to translate this mechanism `utot` creates 3 events in the [TChecker] model:
1. `a_emit` that represents `a!`
2. `a_recv` that represents `a?`
3. `stuck`, a special event used to block events when necessary.

For each channel _a_, there are two situations:
* There exist two distinct processes, say _E_ and _R_, that synchronized on _a_ using respectively the labels, `e!` and `e?`. In this case, the following synchronization vector is generated:
```
sync : E@e_emit : R@e_recv
```
  
* Else, if either `E.e!` or `R.e?` has no synchronization counterpart then a fake process and a synchronization is created to block the event. For instance, if `E.e!` exists but there is no process with an edge labelled with `e?`, then `utot` produces something like:
```
process: Stuck
event: nosync
location:Stuck:sink{initial:}
sync: E@e_emit : Stuck@nosync
```
    
The following model describes two processes, an emitter _E_ and an receiver _R_. Both processes synchronize on a channel _a_. Besides, process _E_ emits on another channel _b_ but there is no other process to synchronize on that channel.
```
chan a, b;
process E () {
    state s0, s1; init s0;
    trans s0 -> s1 { sync a!; },
    s1 -> s0 { sync b!; }; 
}
process R () {
    state s0; init s0;
    trans s0 -> s0 { sync a?; };
}
system E, R;
```
    
The output generated with `utot` is the following (note that `b!` is blocked using the additional process `Stuck`):
```
process:E
location:E:s0{initial:}
location:E:s1{}
event:a_emit
edge:E:s0:s1:a_emit{}
event:b emit
edge:E:s1:s0:b_emit{}

process:R
location:R:s0{initial:}
event:a recv
edge:R:s0:s0:a_recv{}
sync:E@aemit:R@a_recv

process:Stuck
event:nosync
location:Stuck:sink{initial:}
sync:E@b_emit:Stuck@nosync
```

### Broadcast synchronization

The broadcast is one to _N_ synchronization mode which means that an emitter send a message and zero, one or more other processes may receive the message.  Here, only receivers are blocked by the absence of an emitted message and the translation for this case is the same than in the [previous section](#ccs-like-synchronization). Furthermore, events of receivers are marked as weakly synchronized in the [TChecker] model using a `?` (see [The `sync` declaration](https://github.com/ticktac-project/tchecker/wiki/TChecker-file-format#the-sync-declaration)).

## Selections

[Uppaal] allows to use iterators in edges by using `select` statements. These statements permit to define local constants that are enumerated. This enumeration yields an instance of the embodying edge for each value of the constant; if several iterators are used then the Cartesian product of domains is used. `utot` makes this enumeration explicit. The following example with only one edge:
```
typedef int [1,2] pid_t;
broadcast chan a[pid_t];
process Template () {
    state s0; init s0;
    trans s0 -> s0 { select i : pid_t; sync a[i]!; };
}

system Template;
```

is translated as a process with two edges by the enumeration of values belonging to type `pid_t`: 
```
process:Template
location:Template:s0{initial:}
# edge with selection: i = 1
event:a_1_emit
edge:Template:s0:s0:a_1_emit{}
# edge with selection: i = 2
event:a_2_emit
edge:Template:s0:s0:a_2_emit{}
```

## Unsupported features

Beside limitations given in above sections, it remains other features that are not translated by `utot`:
 
* The [Uppaal] query language
* Urgent channels
* C-like functions 
* Priorities (channels or processes)
* ... 

[Uppaal]: http://www.uppaal.org "Uppaal"
[utap]: http://people.cs.aau.dk/%7Eadavid/utap/syntax.html "Uppaal syntax"
[TChecker]: http://github.com/ticktac-project/tchecker "TChecker"
[tcksyntax]: https://github.com/ticktac-project/tchecker/wiki/TChecker-file-format "TChecker File Format"
