module top(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18, x19,  o);
input x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18, x19;
output o;
wire x0_c, x1_c, x2_c, x3_c, x4_c, x5_c, x6_c, x7_c, x8_c, x9_c, x10_c, x11_c, x12_c, x13_c, x14_c, x15_c, x16_c, x17_c, x18_c, x19_c;
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
assign x15_c = ~x15;
assign x16_c = ~x16;
assign x17_c = ~x17;
assign x18_c = ~x18;
assign x19_c = ~x19;
and (w0, x0, x2, x5_c, x6, x8, x9_c, x10, x12_c, x17, x18_c, x19_c);
and (w1, x0_c, x4_c, x6_c, x7, x8, x11_c, x13_c);
and (w2, x2, x4, x5_c, x6, x15, x17_c, x18);
and (w3, x1, x2_c, x4, x5_c, x7_c, x8_c, x9_c, x10, x11_c, x12_c, x13_c, x16_c, x17_c, x18);
and (w4, x0_c, x1_c, x2, x3_c, x4_c, x5_c, x6_c, x7_c, x8_c, x9_c, x10, x11_c, x12, x13, x14, x15_c, x18_c, x19_c);
and (w5, x0, x1_c, x2_c, x3, x4, x5_c, x6, x8, x9, x10_c, x11_c, x13, x14, x15_c, x16_c, x17_c, x18, x19);
and (w6, x1_c, x4_c, x5, x7_c, x9, x10, x11, x12, x13_c, x14, x15_c, x16, x18, x19_c);
and (w7, x0, x2, x3, x4_c, x5_c, x6, x7_c, x8, x9_c, x10, x11_c, x13, x15, x16_c, x17, x18, x19_c);
and (w8, x3_c, x7_c, x8, x10, x11, x12, x13_c, x14_c, x19_c);
and (w9, x0_c, x1_c, x2_c, x3_c, x4, x5, x6_c, x7, x8, x9, x10_c, x11, x12_c, x13, x14_c, x15, x16, x17, x18, x19);
and (w10, x2, x3, x6_c, x8, x11, x18_c);
and (w11, x2, x3, x4_c, x5, x8_c, x9, x12_c, x14_c, x15, x16, x18, x19_c);
and (w12, x0, x1, x2_c, x3_c, x4, x5_c, x6_c, x7, x8_c, x9, x11_c, x12, x13, x14_c, x15, x16_c, x17_c, x18, x19);
and (w13, x1, x3, x4_c, x6_c, x7_c, x8, x10_c, x11, x12_c, x13, x14, x16, x17_c, x18, x19_c);
and (w14, x4_c, x5, x6, x13_c, x16, x19_c);
and (w15, x0, x2_c, x4_c, x5_c, x6_c, x8, x11, x12_c, x14_c, x15_c, x16, x17);
and (w16, x2, x4, x5, x6, x7_c, x8, x9_c, x10, x11_c, x12, x13_c, x14, x15_c, x17, x18, x19_c);
assign w17 = x7_c;
and (w18, x0, x1, x2_c, x3_c, x4_c, x5_c, x6_c, x7, x9, x11, x12, x13_c, x14, x15, x16_c, x17, x18_c, x19_c);
and (w19, x0_c, x1_c, x4, x13, x14_c, x15_c, x18);
xor (o, w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15, w16, w17, w18, w19);
endmodule