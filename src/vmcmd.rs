pub enum VMCmd {
    Add,
    Sub,
    Neg,
    Eq,
    Gt,
    Lt,
    And,
    Or,
    Not,
    Pop(PopCmd),
    Push(PushCmd),
    Return,
}

pub struct PushCmd {
    pub segment: Segment,
    pub value: i16,
}

pub struct PopCmd {
    pub segment: Segment,
    pub value: i16,
}

pub enum Segment {
    Argument,
    Local,
    Static,
    Constant,
    This,
    That,
    Pointer,
    Temp,
}
