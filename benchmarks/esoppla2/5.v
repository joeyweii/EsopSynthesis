module top(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14,  o);
input x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14;
output o;
wire x0_c, x1_c, x2_c, x3_c, x4_c, x5_c, x6_c, x7_c, x8_c, x9_c, x10_c, x11_c, x12_c, x13_c, x14_c;
wire w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15, w16, w17, w18, w19;
assign x0_c = ~x0;
assign x1_c = ~x1;
assign x2_c = ~x2;
assign x3_c = ~x3;
assign x4_c = ~x4;
assign x5_c = ~x5;
assign x6_c = ~x6;
assign x7_c = ~x7;
assign x8_c = ~x8;
assign x9_c = ~x9;
assign x10_c = ~x10;
assign x11_c = ~x11;
assign x12_c = ~x12;
assign x13_c = ~x13;
assign x14_c = ~x14;
assign w0 = x7;
and (w1, x1_c, x2, x4, x8_c, x11, x14_c);
and (w2, x7_c, x14_c);
and (w3, x0_c, x2_c, x3_c, x4, x5, x6_c, x7_c, x8_c, x9, x11, x12_c, x13);
and (w4, x9_c, x14);
and (w5, x1, x2_c, x5, x13_c);
and (w6, x0_c, x1_c, x2, x3_c, x4_c, x5_c, x6_c, x7_c, x8, x9, x10_c, x11_c, x12, x13_c);
and (w7, x0_c, x1_c, x2_c, x4_c, x5_c, x6, x7_c, x8, x9, x10, x11_c, x12_c, x13_c, x14);
and (w8, x1_c, x2_c, x3, x4, x5_c, x6, x7, x8, x10_c, x13_c, x14);
and (w9, x0_c, x1_c, x2_c, x5, x7, x9, x12_c, x13_c, x14);
and (w10, x1, x2, x5, x8, x11_c, x13, x14_c);
and (w11, x0, x2_c, x3_c, x5, x7, x8, x9_c, x11_c, x13);
and (w12, x1, x4_c, x7, x8_c, x9, x11, x12_c);
and (w13, x0, x1_c, x2_c, x6_c, x10_c, x11, x12, x14);
and (w14, x1_c, x2_c, x3_c, x4, x5_c, x7_c, x8_c, x9_c, x10, x12, x13_c);
and (w15, x2, x5, x6_c, x7, x9_c, x10, x11_c, x12, x13);
and (w16, x7, x9, x13);
and (w17, x0, x1_c, x2, x3, x5_c, x6, x11, x12, x13_c);
and (w18, x0_c, x2, x3_c, x4_c, x9, x10);
and (w19, x0_c, x1_c, x2, x3, x8_c, x9_c, x12, x13_c, x14_c);
xor (o, w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15, w16, w17, w18, w19);
endmodule
