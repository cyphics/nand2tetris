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
    Return(u32),
}

pub struct PushCmd {
    pub segment: String,
    pub value: i16,
}

pub struct PopCmd {
    pub segment: String,
    pub value: i16,
}
