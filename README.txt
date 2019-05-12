Samuel Woodruff 
Project 3
3/21/19

I modified my project so if n wasn't given then it will be set 
to how many words are in the file. This was to ensure all words would get read by default.
If n is specified then n child processes will happen. Each child will write a total of 
5 indexes to either palin.out or nopalin.out.

Words from inputfile can be seperated by any amount of whitespace(\t,\n, ' ')

-s will be set with default of 2 or can be specified as an argument to set how many active
children can exist at once.

-h will show options and tell the user the defaults

I am turning this in a second time because the first time I used pthread mutex in shared mem. 
I wanted to try to use semaphore.h and was able to successfully get it working.
I'd also like to make note that my project outputs to the files wrong if there isn't at least
five words present in the file.
