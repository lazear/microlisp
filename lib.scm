(define caar (lambda(a) (car (car a))))
(define cadr (lambda(a) (car (cdr a))))
(define cdar (lambda(a) (cdr (car a))))
(define cddr (lambda(a) (cdr (cdr a))))
(define cadar (lambda(a) (car (cdr (car a)))))
(define caddr (lambda(a) (car (cdr (cdr a)))))
(define cdddr (lambda(a) (cdr (cdr (cdr a)))))
(define cadddr (lambda(a) (car (cdr (cdr (cdr a))))))

;;; Various tests
(define x '(1 2 3))
(define (func args) 
	(begin
		(set! args (cons args 2))
		(cdr args)
	)
)
(define map (lambda (f a)
	(if (null? a) 
		'()
		(cons (f (car a)) (map f (cdr a)))
	)
))

(define (sum-of-squares num-list)
	(define sos-helper (lambda (remaining sum-so-far)
		(if (null? remaining) sum-so-far (sos-helper (cdr remaining) (+ (* (car remaining) (car remaining)))
			))))
	(sos-helper num-list 0)
)
(define factorial (lambda(n) (if (= n 0) 1 (* n (factorial (- n 1))))))
(define add1 (lambda(n) (+ 1 n)))

(define (branch num) 
	(if (= num 0) 'zero 
	(if (= num 1) 'one 
	(if (= num 2) 'two 
		"i can't count higher"))))

(define (assoc key list)
	(if (null? list)
		'()
		(if (eq key (car (car list))) 
			(cdr (car list))
			(assoc key (cdr list))
		)
	)
)