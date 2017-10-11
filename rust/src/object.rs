use std::fmt::{Display, Formatter};
use gc::*;

type ObjectRef = Gc<Object>;

#[derive(Debug, PartialEq, Clone)]
pub enum Object {
    Integer(i64),
    Decimal(f64),
    Literal(String),
    Symbol(String),
    List(ObjectRef, ObjectRef),
    Null,
}

impl Trace for Object {
    fn trace(&self) -> Option<Vec<ObjectRef>> {
        match self {
            // Only List type has reachable references for now
            &Object::List(ref head, ref tail) => {
                let mut reachable: Vec<ObjectRef> = Vec::new();
                reachable.push(*head);
                reachable.push(*tail);
                // Recurse through the head and tail for references
                if let Some(mut refs) = head.trace() {
                    reachable.append(&mut refs);
                
                };
                if let Some(mut refs) = tail.trace() {
                    reachable.append(&mut refs);
                };
                Some(reachable)
            },
            _ => None,
        }
    }
}

impl Display for Object {
     fn fmt(&self, f: &mut Formatter) -> ::std::fmt::Result {
         match self {
             &Object::Integer(x) => write!(f, "{}", x),
             &Object::Decimal(x) => write!(f, "{}", x),
             &Object::Literal(ref x) => write!(f, "{}", x),
             &Object::Symbol(ref x) => write!(f, "{}", x),
             &Object::List(ref head, ref tail) => write!(f, "({}, {})", **head, **tail),
             &Object::Null => write!(f, "'()"),
         }
     }
}

impl Drop for Object {
    fn drop(&mut self) {
        println!("Dropping object {}", self);
    }
}


impl Heap<Object> {
    pub fn allocate_integer(&mut self, data: i64) -> Gc<Object> {
        self.allocate(Object::Integer(data))
    }

    pub fn allocate_decimal(&mut self, data: f64) -> Gc<Object> {
        self.allocate(Object::Decimal(data))
    }

    pub fn allocate_literal(&mut self, data: String) -> Gc<Object> {
        self.allocate(Object::Literal(data))
    }

    pub fn allocate_symbol(&mut self, data: String) -> Gc<Object> {
        self.allocate(Object::Symbol(data))
    }

    pub fn allocate_list(&mut self, head: Gc<Object>, tail: Gc<Object>) -> Gc<Object> {
        self.allocate(Object::List(head, tail))
    }

    pub fn allocate_null(&mut self) -> Gc<Object> {
        self.allocate(Object::Null)
    }
}

impl Object {
    fn car(&self) -> Result<ObjectRef, String> {
        match self {
            &Object::List(head, _) => Ok(head),
            _ => Err("function car expects list".into()),
        }
    }

    fn cdr(&self) -> Result<ObjectRef, String> {
        match self {
            &Object::List(_, tail) => Ok(tail),
            _ => Err("function car expects list".into()),
        }
    }

    fn set_car(&self, other: ObjectRef) -> Result<ObjectRef, String> {
        match self {
            &Object::List(head, _) => {
                head.swap((*other).clone());
                Ok(head)
            }
            _ => Err("function car expects list".into()),
        }
    }

    fn set_cdr(&self, other: ObjectRef) -> Result<ObjectRef, String> {
        match self {
            &Object::List(_, tail) => {
                tail.swap((*other).clone());
                Ok(tail)
            }
            _ => Err("function car expects list".into()),
        }
    }
}



#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn mark_cons() {
        let mut heap: Heap<Object> = Heap::new();
        let a = heap.allocate_integer(10);
        let b = heap.allocate_integer(12);
        let c = heap.allocate_integer(42);
        let cons = heap.allocate_list(a, b);
        cons.mark();
        assert_eq!(cons.is_marked(), true);
        assert_eq!(a.is_marked(), true);
        assert_eq!(b.is_marked(), true);
        assert_eq!(c.is_marked(), false);
    }

    #[test]
    fn swap() {
        let mut heap: Heap<Object> = Heap::new();
        let a = heap.allocate_integer(10);

        assert_eq!(*a, Object::Integer(10));
        assert_eq!(a.swap(Object::Decimal(10.0)), Object::Integer(10));
        assert_eq!(*a, Object::Decimal(10.0));
    }

    #[test]
    fn car() {
        let mut heap: Heap<Object> = Heap::new();
        let a = heap.allocate_integer(10);
        let b = heap.allocate_integer(12);
        let cons = heap.allocate_list(a, b);
        assert_eq!(cons.car(), Ok(a));
        assert!(a.car().is_err());
    }

    #[test]
    fn cdr() {
        let mut heap: Heap<Object> = Heap::new();
        let a = heap.allocate_integer(10);
        let b = heap.allocate_integer(12);
        let cons = heap.allocate_list(a, b);
        assert_eq!(cons.cdr(), Ok(b));
    }

    #[test]
    fn set_car() {
        let mut heap: Heap<Object> = Heap::new();
        let a = heap.allocate_integer(10);
        let b = heap.allocate_integer(12);
        let cons = heap.allocate_list(a, b);
        cons.set_car(b);
        assert_eq!(cons.car(), Ok(b));
    }

    #[test]
    fn set_cdr() {
        let mut heap: Heap<Object> = Heap::new();
        let a = heap.allocate_integer(10);
        let b = heap.allocate_integer(12);
        let cons = heap.allocate_list(a, b);
        assert_eq!(cons.set_cdr(a), Ok(b));
        assert_eq!(cons.cdr(), Ok(b));
    }
}