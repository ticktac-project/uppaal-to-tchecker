# uppaal-to-tchecker limitations

`utot` is not able to compile all [Uppaal] models. In this document we give 
details on supported and unsupported features of Uppaal input format.

## Supported features

### Basic types

Except clocks, which remain unchanged, basic types of [Uppaal] are translated as 
integer variables in [TChecker]. For instance, following variables:

    int n;
    int [-1,2] m;
    const int [-1,2] M = 2; 
    bool is_true = true; 
    scalar[2] S;
    clock C; 

are translated as:

    # int:array-size:min-value:max-value:initial-value:identifier 
    int:1:-32768:32767:0:n
    int:1:-1:2:0:m 
    int:1:2:2:2:M 
    int:1:0:1:1:is_true
    int:1:0:1:0:S
    
    # clock:array-size:identifier 
    clock:1:C

### Arrays

Only arrays supported by [TChecker] are translated as is, that is to say, 
one-dimension arrays indexed from 0 and with the cells initialized with the same 
value. In other cases, `utot` flatten arrays and split them in individual 
variables. For instance, the following two-dimension array of scalar values

    typedef int [-1,4] sometype t;
    sometype t tab2[2][3] = { { -1, 0, 1 }, { 2, 3, 4 } };

is translated as 6 variables (one per cell):

    int:1:-1:4:-1:tab2_0_0
    int:1:-1:4:0:tab2_0_1
    int:1:-1:4:1:tab2_0_2
    int:1:-1:4:2:tab2_1_0
    int:1:-1:4:3:tab2_1_1
    int:1:-1:4:4:tab2_1_2

In the next example, three 1-dimension arrays are declared. 

    typedef int [-1,2] sometype t; 
    sometype t tab1[2];
    sometype t tab2[2] = { 1, 1 }; 
    sometype t tab3[2] = { 0, 1 };

The third one is initialized with different values for cells; so it is split into separate  
variables:

    int:2:-1:2:0:tab1
    int:2:-1:2:1:tab2
    int:1:-1:2:0:tab3_0
    int:1:-1:2:1:tab3_1

In the case of arrays with shifted indices, as for instance the array `values`
below:

    typedef int [999, 1005] pid t; int [0,3] values[pid t];
    pid t i;
    process P () {
        state s0, s1 { values[i] < 2 };
        init s0;
    }

the offset of cells is explicitly computed in each expression where the array 
cells occur (here in the invariant of location `s1`):

    int:7:0:3:0:values
    int:1:999:1005:999:i
    process:P
    location:P:s0{initial:}
    location:P:s1{invariant:(1 && (values[i-999] < 2))}

### Templates

[Uppaal] allows the declaration of class of timed automata called _templates_. 
These templates might be parameterized with constants, variables and even 
reference to variables. 

In [Tchecker] input format, all instances of templates must be declared 
explicitly. 
In order to enumerate all instances of parameters, `utot` tries to substitute 
parameters with their actual value. If during the instantiation process an 
expected constant can not be evaluated then an error is generated. 

`utot` supports partial instantiation of templates which means that
it enumerates all possible values of parameter with unspecified value. Each 
instance is assigned an identifier built from the template identifier follow by
couples _param_\__value_ for each parameter _param_ and each possible value for 
_param_.

For instance, the following model

    typedef int [1,2] pid t;
    typedef scalar[3] key t;
    process Process(const pid t pid, key t k) { }
    system Process;

is translated into 6 instances of the process `Process` as :

    process:Process_k_0_pid_1
    process:Process_k_0_pid_2
    process:Process_k_1_pid_1
    process:Process_k_1_pid_2
    process:Process_k_2_pid_1
    process:Process_k_2_pid_2
 
For each instance of a template its local elements are duplicated. Variables
and clocks are prefixed with the identifier of the instance:

    process Q() { bool x; state s0; init s0; }
    P1 := Q();
    P2 := Q();
    system P1, P2;

should yield the following [TChecker] model:

    process:P1
    int:1:0:1:0:P1_x
    location:P1:s0{initial:}
    process:P2
    int:1:0:1:0:P2_x
    location:P2:s0{initial:} 

Finally, parameters are replaced by their values. In the following example, 

    process Proc (clock &c, const int B) { 
        state s0; 
        init s0;
        trans s0 -> s0 { guard c < B; };
    }
    clock x[2];
    S = Proc(x[1], 7);
    system S;

the reference to a clock `c` and the integer constant `B` are replaced in the
instance by their respective values, `x[1]` and `7`:

    clock:2:x
    process:S
    location:S:s0{initial:}
    event:tau edge:S:s0:s0:tau{provided:(x[1] < 7)}

### Synchronizations of events


## Unsupported features

### Query language
### Urgent channels
### C-like functions
### Only TChecker expressions struct types
### Arrays (only partially supported)
### Template parameters are assumed constants Quantifiers in Boolean expressions
### CSP-like synchronization

[Uppaal]: http://www.uppaal.org "Uppaal"
[TChecker]: http://github.com/ticktac-project/tchecker "TChecker"
[utap]: http://people.cs.aau.dk/%7Eadavid/utap/syntax.html "Uppaal syntax"