module top(x0, x1, x2,  o);
input x0, x1, x2;
output o;
wire x0_c, x1_c, x2_c;
wire w0, w1, w2, w3;
assign x0_c = ~x0;
assign x1_c = ~x1;
assign x2_c = ~x2;
assign w0 = x2;
and (w1, x0, x1_c);
and (w2, x0_c, x1);
and (w3, x0_c, x1, x2_c);
xor (o, w0, w1, w2, w3);
endmodule
