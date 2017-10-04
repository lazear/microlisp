extern crate lisp;
use lisp::gc::{Gc, Heap};
use lisp::object::Object;

fn main() {
    let mut heap: Heap<Object> = Heap::new();

    let s = heap.allocate(Object::Literal("lisp".into()));
    let q = heap.allocate(Object::Decimal(6.47573));
    let cons = heap.allocate(Object::List(s, q));
    println!("cons {}", cons);
    heap.mark(&mut vec![cons]);
    
}