module top(x0, x1, x2, x3,  o);
input x0, x1, x2, x3;
output o;
wire x0_c, x1_c, x2_c, x3_c;
wire w0, w1, w2, w3, w4;
assign x0_c = ~x0;
assign x1_c = ~x1;
assign x2_c = ~x2;
assign x3_c = ~x3;
and (w0, x0, x2_c);
and (w1, x0_c, x1_c, x2_c);
and (w2, x0_c, x2_c, x3);
and (w3, x0, x3);
and (w4, x0, x2);
xor (o, w0, w1, w2, w3, w4);
endmodule
