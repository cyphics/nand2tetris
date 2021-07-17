pub enum VMCmd {
    Add(u32),
    Sub(u32),
    Neg(u32),
    Eq(u32),
    Gt(u32),
    Lt(u32),
    And(u32),
    Or(u32),
    Not(u32),
    Pop(PopCmd, u32),
    Push(PushCmd, u32),
    Label(String, u32),
    Goto(String, u32),
    IfGoto(String, u32),
    Function(FunctionCmd, u32),
    Call(CallCmd, u32),
    Return(u32),
}

pub struct FunctionCmd {
    pub function_name: String,
    pub local_args: usize,
}

pub struct CallCmd {
    pub function_name: String,
    pub args: usize,
}

pub struct PushCmd {
    pub segment: String,
    pub value: i16,
}

pub struct PopCmd {
    pub segment: String,
    pub value: i16,
}
