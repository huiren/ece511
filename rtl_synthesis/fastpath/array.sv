module neuron #(parameter WEIGHT_NUM = 33, parameter WEIGHT_ENTRY_NUM = 64)
(
    input clk,
    input write, // 
    input [31:0] last_h,
    input [32*6-1:0] last_v,
    input br_outcome, last_prediction,
    input [5:0] rdIdx, wrIdx, // Max 4096
    output logic [WEIGHT_NUM*8-1:0] dataout // 33 * 9 = 297
);

// 9-bit per weight, 33 weights per branch, 4096 branch records
logic[7:0] data [WEIGHT_NUM][WEIGHT_ENTRY_NUM];
logic[5:0] i0, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10,
           i11, i12, i13, i14, i15, i16, i17, i18, i19, i20,
           i21, i22, i23, i24, i25, i26, i27, i28, i29, i30,
           i31;



always_ff @(posedge clk)
begin
	// Self update involved
    if (write == 1 & (br_outcome != last_prediction))
    begin
        if (br_outcome == 1)
            data[0][wrIdx] <= data[0][wrIdx] + 8'd1;
        else
            data[0][wrIdx] <= data[0][wrIdx] - 8'd1;
            
    	data[1][i0] <= (br_outcome == last_h[0] ? data[1][i0] + 8'd1: data[1][i0] - 8'd1);
        data[2][i1] <= (br_outcome == last_h[1] ? data[2][i1] + 8'd1: data[2][i1] - 8'd1);
        data[3][i2] <= (br_outcome == last_h[2] ? data[3][i2] + 8'd1: data[3][i2] - 8'd1);
        data[4][i3] <= (br_outcome == last_h[3] ? data[4][i3] + 8'd1: data[4][i3] - 8'd1);
        data[5][i4] <= (br_outcome == last_h[4] ? data[5][i4] + 8'd1: data[5][i4] - 8'd1);
        data[6][i5] <= (br_outcome == last_h[5] ? data[6][i5] + 8'd1: data[6][i5] - 8'd1);
        data[7][i6] <= (br_outcome == last_h[6] ? data[7][i6] + 8'd1: data[7][i6] - 8'd1);
        data[8][i7] <= (br_outcome == last_h[7] ? data[8][i7] + 8'd1: data[8][i7] - 8'd1);
        data[9][i8] <= (br_outcome == last_h[8] ? data[9][i8] + 8'd1: data[9][i8] - 8'd1);
        data[10][i9] <= (br_outcome == last_h[9] ? data[10][i9] + 8'd1: data[10][i9] - 8'd1);
        data[11][i10] <= (br_outcome == last_h[10] ? data[11][i10] + 8'd1: data[11][i10] - 8'd1);
        data[12][i11] <= (br_outcome == last_h[11] ? data[12][i11] + 8'd1: data[12][i11] - 8'd1);
        data[13][i12] <= (br_outcome == last_h[12] ? data[13][i12] + 8'd1: data[13][i12] - 8'd1);
        data[14][i13] <= (br_outcome == last_h[13] ? data[14][i13] + 8'd1: data[14][i13] - 8'd1);
        data[15][i14] <= (br_outcome == last_h[14] ? data[15][i14] + 8'd1: data[15][i14] - 8'd1);
        data[16][i15] <= (br_outcome == last_h[15] ? data[16][i15] + 8'd1: data[16][i15] - 8'd1);
        data[17][i16] <= (br_outcome == last_h[16] ? data[17][i16] + 8'd1: data[17][i16] - 8'd1);
        data[18][i17] <= (br_outcome == last_h[17] ? data[18][i17] + 8'd1: data[18][i17] - 8'd1);
        data[19][i18] <= (br_outcome == last_h[18] ? data[19][i18] + 8'd1: data[19][i18] - 8'd1);
        data[20][i19] <= (br_outcome == last_h[19] ? data[20][i19] + 8'd1: data[20][i19] - 8'd1);
        data[21][i20] <= (br_outcome == last_h[20] ? data[21][i20] + 8'd1: data[21][i20] - 8'd1);
        data[22][i21] <= (br_outcome == last_h[21] ? data[22][i21] + 8'd1: data[22][i21] - 8'd1);
        data[23][i22] <= (br_outcome == last_h[22] ? data[23][i22] + 8'd1: data[23][i22] - 8'd1);
        data[24][i23] <= (br_outcome == last_h[23] ? data[24][i23] + 8'd1: data[24][i23] - 8'd1);
        data[25][i24] <= (br_outcome == last_h[24] ? data[25][i24] + 8'd1: data[25][i24] - 8'd1);
        data[26][i25] <= (br_outcome == last_h[25] ? data[26][i25] + 8'd1: data[26][i25] - 8'd1);
        data[27][i26] <= (br_outcome == last_h[26] ? data[27][i26] + 8'd1: data[27][i26] - 8'd1);
        data[28][i27] <= (br_outcome == last_h[27] ? data[28][i27] + 8'd1: data[28][i27] - 8'd1);
        data[29][i28] <= (br_outcome == last_h[28] ? data[29][i28] + 8'd1: data[29][i28] - 8'd1);
        data[30][i29] <= (br_outcome == last_h[29] ? data[30][i29] + 8'd1: data[30][i29] - 8'd1);
        data[31][i30] <= (br_outcome == last_h[30] ? data[31][i30] + 8'd1: data[31][i30] - 8'd1);
        data[32][i31] <= (br_outcome == last_h[31] ? data[32][i31] + 8'd1: data[32][i31] - 8'd1);

    end 
end

assign i0 = last_v[5:0];
assign i1 = last_v[11:6];
assign i2 = last_v[17:12];
assign i3 = last_v[23:18];
assign i4 = last_v[29:24];
assign i5 = last_v[35:30];
assign i6 = last_v[41:36];
assign i7 = last_v[47:42];
assign i8 = last_v[53:48];
assign i9 = last_v[59:54];
assign i10 = last_v[65:60];
assign i11 = last_v[71:66];
assign i12 = last_v[77:72];
assign i13 = last_v[83:78];
assign i14 = last_v[89:84];
assign i15 = last_v[95:90];
assign i16 = last_v[101:96];
assign i17 = last_v[107:102];
assign i18 = last_v[113:108];
assign i19 = last_v[119:114];
assign i20 = last_v[125:120];
assign i21 = last_v[131:126];
assign i22 = last_v[137:132];
assign i23 = last_v[143:138];
assign i24 = last_v[149:144];
assign i25 = last_v[155:150];
assign i26 = last_v[161:156];
assign i27 = last_v[167:162];
assign i28 = last_v[173:168];
assign i29 = last_v[179:174];
assign i30 = last_v[185:180];
assign i31 = last_v[191:186];


assign dataout[7:0] = data[0][rdIdx];
assign dataout[15:8] = data[1][rdIdx];
assign dataout[23:16] = data[2][rdIdx];
assign dataout[31:24] = data[3][rdIdx];
assign dataout[39:32] = data[4][rdIdx];
assign dataout[47:40] = data[5][rdIdx];
assign dataout[55:48] = data[6][rdIdx];
assign dataout[63:56] = data[7][rdIdx];
assign dataout[71:64] = data[8][rdIdx];
assign dataout[79:72] = data[9][rdIdx];
assign dataout[87:80] = data[10][rdIdx];
assign dataout[95:88] = data[11][rdIdx];
assign dataout[103:96] = data[12][rdIdx];
assign dataout[111:104] = data[13][rdIdx];
assign dataout[119:112] = data[14][rdIdx];
assign dataout[127:120] = data[15][rdIdx];
assign dataout[135:128] = data[16][rdIdx];
assign dataout[143:136] = data[17][rdIdx];
assign dataout[151:144] = data[18][rdIdx];
assign dataout[159:152] = data[19][rdIdx];
assign dataout[167:160] = data[20][rdIdx];
assign dataout[175:168] = data[21][rdIdx];
assign dataout[183:176] = data[22][rdIdx];
assign dataout[191:184] = data[23][rdIdx];
assign dataout[199:192] = data[24][rdIdx];
assign dataout[207:200] = data[25][rdIdx];
assign dataout[215:208] = data[26][rdIdx];
assign dataout[223:216] = data[27][rdIdx];
assign dataout[231:224] = data[28][rdIdx];
assign dataout[239:232] = data[29][rdIdx];
assign dataout[247:240] = data[30][rdIdx];
assign dataout[255:248] = data[31][rdIdx];
assign dataout[263:256] = data[32][rdIdx];

endmodule : neuron
