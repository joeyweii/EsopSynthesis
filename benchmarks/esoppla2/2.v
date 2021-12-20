module top(x0, x1, x2, x3,  o);
input x0, x1, x2, x3;
output o;
wire x0_c, x1_c, x2_c, x3_c;
wire w0, w1, w2;
assign x0_c = ~x0;
assign x1_c = ~x1;
assign x2_c = ~x2;
assign x3_c = ~x3;
and (w0, x0_c, x1_c, x3_c);
and (w1, x0, x1_c, x2_c, x3_c);
and (w2, x0_c, x1, x2_c, x3_c);
xor (o, w0, w1, w2);
endmodule
