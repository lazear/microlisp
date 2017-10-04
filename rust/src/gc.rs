#![allow(dead_code)]

use std::marker::PhantomData;
use std::ops::{Deref, DerefMut};

pub trait Trace : Sized {
    fn trace(&self) -> Option<Vec<Gc<Self>>>;
}

/// Structure containing actual data, as well as
/// relevant information (whether item is rooted and marked)
pub struct HeapBox<T: Trace> {
    marked: bool,
    rooted: bool,
    data: T,
}

/// A pool of allocated objects tracked by the 
/// garbage collector
pub struct Heap<T: Trace> {
    pool: Vec<Box<HeapBox<T>>>,
}

/// A pointer to a GC allocated object
#[derive(Debug)]
pub struct Gc<T: Trace> {
    ptr: *mut HeapBox<T>,
    _pd: PhantomData<T>,
}

impl<T: Trace> Gc<T> {
    pub fn mark(&self) {
        unsafe {
            (*self.ptr).marked = true;
            if let Some(reachable) = self.trace() {
                for mut obj in reachable.into_iter() {
                    obj.mark();
                }
            }
        }
    }

    pub fn is_marked(&self) -> bool {
        unsafe {
            (*self.ptr).marked
        }
    }

    pub fn swap(&self, data: T) -> T {
        unsafe {
            ::std::mem::replace(&mut (*self.ptr).data, data)
        }
    }
}

impl<T: Trace> Deref for Gc<T> {
    type Target = T;
    fn deref(&self) -> &T {
        unsafe {
            &(*self.ptr).data
        }
    }
}

impl<T: Trace> DerefMut for Gc<T> {
    fn deref_mut(&mut self) -> &mut T {
        unsafe {
            &mut (*self.ptr).data
        }
    }
}

impl <T: Trace> Clone for Gc<T> {
    fn clone(&self) -> Self {
        *self
    }
}

impl<T: Trace> Copy for Gc<T> {}


impl<T: Trace> Heap<T> {
    pub fn new() -> Self {
        Heap {
            pool: Vec::new()
        }
    }

    pub fn allocate(&mut self, data: T) -> Gc<T> {
        let mut heapbox = Box::new(HeapBox {
            marked: false,
            rooted: false,
            data: data,
        });
        let gc = Gc {
            ptr: heapbox.as_mut(),
            _pd: PhantomData,
        };
        self.pool.push(heapbox);
        gc
    }

    pub fn sweep(&mut self) -> usize {
        let mut swept: Vec<usize> = Vec::new();
        
        for (idx, obj) in self.pool.iter().enumerate() {
            if !obj.marked {
                swept.push(idx);
            }
        }

        let ret = swept.len();
        // We need to use pop() so that we delete objects
        // from the back of the pool to the front.
        // If we start from the front, then the indexes will be off
        // ... There is surely a better way to do this
        while let Some(idx) = swept.pop() {
            self.pool.remove(idx);
        };
        ret
    }

    pub fn mark(&self, roots: &mut Vec<Gc<T>>) {
        for root in roots.into_iter() {
            root.mark();
            //  match root.trace() {
            //     Some(reachable) => {
            //         for mut obj in reachable.into_iter() {
            //             obj.mark();
            //         }
            //     },
            //     None => (),
            // }            
        }
    }
}

impl<T: Trace + ::std::fmt::Display> ::std::fmt::Display for Gc<T> {
    fn fmt(&self, f: &mut ::std::fmt::Formatter) -> ::std::fmt::Result {
        write!(f, "{}", self.deref())
    }
}

impl<T: Trace + PartialEq> PartialEq for Gc<T> {
    fn eq(&self, other: &Self) -> bool {
        self.deref() == other.deref()
    }
}

