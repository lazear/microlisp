;;; Michael Lazear, (C) 2016 - MIT License
;;; Standard Library functions

(define caar (lambda(a) (car (car a))))
(define cadr (lambda(a) (car (cdr a))))
(define cdar (lambda(a) (cdr (car a))))
(define cddr (lambda(a) (cdr (cdr a))))
(define cadar (lambda(a) (car (cdr (car a)))))
(define caddr (lambda(a) (car (cdr (cdr a)))))
(define cdddr (lambda(a) (cdr (cdr (cdr a)))))
(define cadddr (lambda(a) (car (cdr (cdr (cdr a))))))

(define != (lambda (a b) (if (= a b) #f #t)))
(define >= (lambda (a b) (if (< a b) #f #t)))
(define <= (lambda (a b) (if (> a b) #f #t)))

;;; Return the maximum value from a list of integers
(define (max list-of-numbers)
  (define (max-iter best remaining)
    (cond ((null? remaining) best)
      ((> (car remaining) best) (max-iter (car remaining) (cdr remaining)))
      (else (max-iter best (cdr remaining)))))
  (max-iter (car list-of-numbers) (cdr list-of-numbers)))

;;; Return the minimum value from a list of integers
(define (min list-of-numbers)
  (define (min-iter best remaining)
    (cond ((null? remaining) best)
      ((< (car remaining) best) (min-iter (car remaining) (cdr remaining)))
      (else (min-iter best (cdr remaining)))))
  (min-iter (car list-of-numbers) (cdr list-of-numbers)))

;;; Map a function 'f' onto list 'a'
(define map (lambda (f a)
    (if (null? a) 
      '()
      (cons (f (car a)) (map f (cdr a))))))


;;; Provide the association pair of key from list
(define (assoc key list)
  (if (null? list)
    '()
    (if (eq? key (car (car list))) 
      (car list)
      (assoc key (cdr list)))))

;;; Lambda key-list with dispatch
(define (make-key-list)
  (let ((list '())) 
    (define get-val (lambda (var) 
        (assoc var list)))
    (define add-key (lambda (var val)
        (set! list (cons (cons var val) list))))
    (define (dispatch m)
      (if (eq? m 'add) add-key
        (if (eq? m 'get) get-val 
          list)))
    dispatch))

;;; Lambda stack with dispatch
(define (make-stack)
  (let ((stack '()))
    (define push (lambda (x) 
        (set! stack (cons x stack))
        stack))
    (define pop (lambda (x)
        (define q (car stack))
        (set! stack (cdr stack))
        q))
    (define (dispatch m)
      (if (eq? m 'push) push
        (if (eq? m 'pop) pop 
          stack)))
    dispatch))
;;; Returns the last item in a list or pair.
;;; Pointer to cdr if list, Pointer to object if pair
(define last-item-in-list (lambda (list)
    (define (helper remaining)
      (if (null? (cdr remaining))
        remaining
        (helper (cdr remaining))))
    (helper list)))

;;; Returns a list from (0-number)
(define (range number) 
  (define (range-helper start max)
    (if (= start max)
      (cons max '())
      (cons start (range-helper (+ 1 start) max))))
  (range-helper 0 number))

;;; Returns a list from (0-number)
(define (range-from start finish)
  (if (= start finish)
    finish
    (cons start (range-from (+ 1 start) finish))))

;;; Tail recursive length
(define (length list)
  (define (length-helper accum remaining)
    (if (null? remaining)
      accum
      (length-helper (+ 1 accum) (cdr remaining))))
  (length-helper 1 (cdr list)))

;;; Append list2 to list1
(define append (lambda (list1 list2) 
    (define (append-helper l1 l2)
      (if (null? l1)
        l2
        (cons (car l1) (append-helper (cdr l1) l2))))
    (append-helper list1 list2)))

;;; Reverse list
(define (reverse list)
  (define (reverse-iter remaining first) 
    (if (null? remaining)
      first
      (reverse-iter (cdr remaining) (cons (car remaining) first))))
  (reverse-iter list '()))

(define (pow num exp) 
  (define (iter a b) 
    (if (eq? b 1) 
      a
      (iter (* a num) (- b 1))))
  (if (eq? exp 0)
    1
    (iter num exp)))

(define (>= a b)
  (if (< a b) #f #t))

(define (<= a b) 
  (if (> a b) #f #t))

(define ge <=)
(define le >=)

(define (mod a b)
  (define (iter rem div)
    (if (< rem div)
      rem
      (iter (- rem div) div)))
  (iter a b))

;;; A couple macros
;;; Because this is LISP and we can...
(define procedure-body (lambda (proc) (caddr proc)))
(define procedure-args (lambda (proc) (cadr proc)))
(define (mutate-procedure-env name new-env) (set-car! (cdddr name) new-env))
(define (mutate-procedure-body name new-body) (set-car! (cddr name) (list new-body)))
(define (mutate-procedure-args name new-args) (set-car! (cdr name) new-args))

(define (construct-procedure args body env)
  (let ((new-proc (cons 'procedure (cons '() (cons '() (cons '()))))))
    (mutate-procedure-args new-proc args)
    (mutate-procedure-body new-proc body)
    (mutate-procedure-env new-proc env)
    new-proc))

(define (if-zero x then) (list 'if (list '= x 0) then))

(define (gen-accum number)
  (lambda (amount)
    (set! number (+ number amount))
    number))

;;; Everytime (new-accum) is called, it's accumulator should be increased by one
(define new-accum (gen-accum 0))

;; Simple for loop
(define for (lambda (start end do)
    (define (for-loop a z)
      (if (= a z)
        'Done;; Last iteration
        (begin 
          (do)
          (for-loop (+ 1 a) z))))
    (for-loop start end do)))

(define (make-withdraw balance)
  (lambda (amount)
    (if (> balance amount)
      (begin (set! balance (- balance amount))
        balance)
      "Insufficient funds")))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Various tests
(define x '(1 2 3))
(define factorial (lambda(n) (if (= n 0) 1 (* n (factorial (- n 1))))))
(define add1 (lambda(n) (+ 1 n)))
(define (sum-of-squares num-list)
  (define sos-helper (lambda (remaining sum-so-far)
      (if (null? remaining) 
        sum-so-far 
        (sos-helper (cdr remaining) (+ sum-so-far (* (car remaining) (car remaining)))))))
  (sos-helper num-list 0))

;;; Procedure with no args
(define (new-env) (cons (cons '() '()) '()))
;;; Construct a procedure with macro
(define new-func (construct-procedure '(a) '(cons a 10) (get-global-environment)))
(define with-macros (construct-procedure '(x) (if-zero 'x 'ZERO) (get-global-environment)))