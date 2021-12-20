module top(x0, x1, x2, x3, x4, x5,  o);
input x0, x1, x2, x3, x4, x5;
output o;
wire x0_c, x1_c, x2_c, x3_c, x4_c, x5_c;
wire w0, w1;
assign x0_c = ~x0;
assign x1_c = ~x1;
assign x2_c = ~x2;
assign x3_c = ~x3;
assign x4_c = ~x4;
assign x5_c = ~x5;
and (w0, x0, x1, x2_c, x4_c, x5);
and (w1, x1, x2_c, x3, x5);
xor (o, w0, w1);
endmodule
