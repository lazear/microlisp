(define (fib n)
  (if (= n 2)
    1
    (if (= n 1)
      1
      (+ 
        (fib (- n 1))
        (fib (- n 2))))))
(print (fib 30))
(exit)