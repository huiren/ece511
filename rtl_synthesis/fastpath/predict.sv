module predict #(parameter WEIGHT_NUM = 33, parameter WEIGHT_WIDTH = 8)
(
	input [WEIGHT_NUM*WEIGHT_WIDTH-1:0] sr,
    input [WEIGHT_NUM*WEIGHT_WIDTH-1:0] w,
    output logic pred,
    output logic [WEIGHT_NUM*WEIGHT_WIDTH-1:0] new_sr,
)

parameter WEIGHT_ENTRY_NUM = 64;

logic [WEIGHT_WIDTH-1:0] sum;

always_comb
begin
	sum = w[WEIGHT_WIDTH-1:0] + sr[WEIGHT_WIDTH-1:0];
	pred = sum[WEIGHT_WIDTH-1];
	if (pred)
	begin
		new_sr[7:0] = sr[15:8] + 8'd1;
		new_sr[15:8] = sr[23:16] + 8'd1;
		new_sr[23:16] = sr[31:24] + 8'd1;
		new_sr[31:24] = sr[39:32] + 8'd1;
		new_sr[39:32] = sr[47:40] + 8'd1;
		new_sr[47:40] = sr[55:48] + 8'd1;
		new_sr[55:48] = sr[63:56] + 8'd1;
		new_sr[63:56] = sr[71:64] + 8'd1;
		new_sr[71:64] = sr[79:72] + 8'd1;
		new_sr[79:72] = sr[87:80] + 8'd1;
		new_sr[87:80] = sr[95:88] + 8'd1;
		new_sr[95:88] = sr[103:96] + 8'd1;
		new_sr[103:96] = sr[111:104] + 8'd1;
		new_sr[111:104] = sr[119:112] + 8'd1;
		new_sr[119:112] = sr[127:120] + 8'd1;
		new_sr[127:120] = sr[135:128] + 8'd1;
		new_sr[135:128] = sr[143:136] + 8'd1;
		new_sr[143:136] = sr[151:144] + 8'd1;
		new_sr[151:144] = sr[159:152] + 8'd1;
		new_sr[159:152] = sr[167:160] + 8'd1;
		new_sr[167:160] = sr[175:168] + 8'd1;
		new_sr[175:168] = sr[183:176] + 8'd1;
		new_sr[183:176] = sr[191:184] + 8'd1;
		new_sr[191:184] = sr[199:192] + 8'd1;
		new_sr[199:192] = sr[207:200] + 8'd1;
		new_sr[207:200] = sr[215:207] + 8'd1;
		new_sr[215:207] = sr[223:215] + 8'd1;
		new_sr[223:215] = sr[231:223] + 8'd1;
		new_sr[231:223] = sr[239:231] + 8'd1;
		new_sr[239:231] = sr[247:239] + 8'd1;		
		new_sr[247:239] = sr[255:247] + 8'd1;
		new_sr[255:247] = sr[263:255] + 8'd1;
		new_sr[263:255] = 8'd0;
	end
	else
	begin
		new_sr[7:0] = sr[15:8] - 8'd1;
		new_sr[15:8] = sr[23:16] - 8'd1;
		new_sr[23:16] = sr[31:24] - 8'd1;
		new_sr[31:24] = sr[39:32] - 8'd1;
		new_sr[39:32] = sr[47:40] - 8'd1;
		new_sr[47:40] = sr[55:48] - 8'd1;
		new_sr[55:48] = sr[63:56] - 8'd1;
		new_sr[63:56] = sr[71:64] - 8'd1;
		new_sr[71:64] = sr[79:72] - 8'd1;
		new_sr[79:72] = sr[87:80] - 8'd1;
		new_sr[87:80] = sr[95:88] - 8'd1;
		new_sr[95:88] = sr[103:96] - 8'd1;
		new_sr[103:96] = sr[111:104] - 8'd1;
		new_sr[111:104] = sr[119:112] - 8'd1;
		new_sr[119:112] = sr[127:120] - 8'd1;
		new_sr[127:120] = sr[135:128] - 8'd1;
		new_sr[135:128] = sr[143:136] - 8'd1;
		new_sr[143:136] = sr[151:144] - 8'd1;
		new_sr[151:144] = sr[159:152] - 8'd1;
		new_sr[159:152] = sr[167:160] - 8'd1;
		new_sr[167:160] = sr[175:168] - 8'd1;
		new_sr[175:168] = sr[183:176] - 8'd1;
		new_sr[183:176] = sr[191:184] - 8'd1;
		new_sr[191:184] = sr[199:192] - 8'd1;
		new_sr[199:192] = sr[207:200] - 8'd1;
		new_sr[207:200] = sr[215:207] - 8'd1;
		new_sr[215:207] = sr[223:215] - 8'd1;
		new_sr[223:215] = sr[231:223] - 8'd1;
		new_sr[231:223] = sr[239:231] - 8'd1;
		new_sr[239:231] = sr[247:239] - 8'd1;		
		new_sr[247:239] = sr[255:247] - 8'd1;
		new_sr[255:247] = sr[263:255] - 8'd1;
		new_sr[263:255] = 8'd0;
	end

end

endmodule : predict