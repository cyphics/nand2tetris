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
    pub segment: String,
    pub value: i16,
}

pub struct PopCmd {
    pub segment: String,
    pub value: i16,
}
