let program = \x, y, z -> x y z;
let program1 = \x -> \y -> \z -> x y z;

let true = \x -> \y -> x;
let false = \x -> \y -> y;

let fst = true;
let snd = false;

let if = \x -> \y -> \f -> f x y;
let pair = if;

let not = \p -> p false true;
let and = \p -> \q -> p q false;
let or  = \p -> \q -> p true q;

let suc = \n -> \f -> \x -> f (n f x);
let add = \n -> \m -> \f -> \x -> m f (n f x);
let mul = \n -> \m -> \f -> \x -> m (n f) x;
let square = \m -> mul m m;

let prefn = \f -> \p -> pair (f (fst p)) (fst p);
let pre = \n -> \f -> \x -> snd (n (prefn f) (pair x x));
let sub = \m -> \n -> n pre m;

let zero = \f -> \x -> x;
let one = \f -> \x -> f x;
let two = \f -> \x -> f (f x);
let three = suc two;
let four = suc three;

let var = add (square three) (square four);
