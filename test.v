module top(a, b, c, d, e, o);
input a, b, c, d, e;
output o;
// wire a_c, b_c, c_c, d_c ,e_c;

wire w1, w2;
and (w1, a, b, c);
and (w2, d, e, c);
xor (o , w1, w2);


endmodule