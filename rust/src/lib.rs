pub mod gc;
pub mod object;




// #[cfg(test)]
// mod tests {
//     use super::*;

//     #[test]
//     fn mark_cons_references() {
//         let mut gc: GcPool<Object> = GcPool::new();
//         let head = gc.allocate(Object::Integer(10));
//         let tail = gc.allocate(Object::Integer(12));
//         let cons = gc.allocate(Object::List(head, tail));
//         let null = gc.allocate(Object::Null);

//         assert_eq!(cons.is_marked(), false);
//         gc.mark(&mut vec![cons]);
//         assert_eq!(cons.is_marked(), true);
//         assert_eq!(head.is_marked(), true);
//         assert_eq!(null.is_marked(), false);
//     }
// }



// fn main() {
//     println!("Hello, world!");

//     let mut gc = Heap::new();
//     let head = gc.allocate_int(10);

//     let tail = gc.allocate_int(12);
//     let _drop = gc.allocate(Object::Integer(2));
//     let cons = gc.allocate(Object::List(head, tail));
//     let _drop2 = gc.allocate(Object::Null);

//     println!("{:?}, {:?}", *cons, cons.is_marked());
//     println!("{:?}", *head);

//     gc.mark(&mut vec![cons]);

//     println!("cons marked? {:?}", cons.is_marked());
//     println!("head marked? {:?}", head.is_marked());

//     gc.sweep();


//     for obj in gc.pool.iter() {
//         println!("obj {:x}", (obj.as_ref()  as *const GcBox<Object>) as usize)
//     };
    
// }


// #[cfg(test)]
// mod tests {
//     #[test]
//     fn it_works() {
//         assert_eq!(2 + 2, 4);
//     }
// }
