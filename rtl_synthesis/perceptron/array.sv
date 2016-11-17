module perceptron #(parameter WEIGHT_NUM = 33, parameter WEIGHT_ENTRY_NUM = 4096)
(
    input clk,
    input write, // 
    input [31:0] ghr,
    input br_outcome,
    input [11:0] rdIdx, wrIdx, // Max 4096
    output logic [WEIGHT_NUM*9-1:0] dataout // 33 * 9 = 297
);

// 9-bit per weight, 33 weights per branch, 4096 branch records
logic[8:0] data [WEIGHT_NUM][WEIGHT_ENTRY_NUM];



/* Initialize array */
initial
begin

    for (int j = 0; j < WEIGHT_ENTRY_NUM; j++)
    begin
    	for (int i = 0; i < WEIGHT_NUM; i++)
    	begin
        	data[i][j] = 9'b0;
       	end
    end
	 
end

always_ff @(posedge clk)
begin
	// Self update involved
    if (write == 1)
    begin
    	for (int k = 0; k < WEIGHT_NUM; k++)
    	begin
    		data[k][wrIdx] <= (br_outcome == ghr[k] ? data[k][wrIdx] + 9'd1: data[k][wrIdx] - 9'd1);
    	end
    end 
end


assign dataout[8:0] = data[0][rdIdx];
assign dataout[17:9] = data[1][rdIdx];
assign dataout[26:18] = data[2][rdIdx];
assign dataout[35:27] = data[3][rdIdx];
assign dataout[44:36] = data[4][rdIdx];
assign dataout[53:45] = data[5][rdIdx];
assign dataout[62:54] = data[6][rdIdx];
assign dataout[71:63] = data[7][rdIdx];
assign dataout[80:72] = data[8][rdIdx];
assign dataout[89:81] = data[9][rdIdx];
assign dataout[98:90] = data[10][rdIdx];
assign dataout[107:99] = data[11][rdIdx];
assign dataout[116:108] = data[12][rdIdx];
assign dataout[125:117] = data[13][rdIdx];
assign dataout[134:126] = data[14][rdIdx];
assign dataout[143:135] = data[15][rdIdx];
assign dataout[152:144] = data[16][rdIdx];
assign dataout[161:153] = data[17][rdIdx];
assign dataout[170:162] = data[18][rdIdx];
assign dataout[179:171] = data[19][rdIdx];
assign dataout[188:180] = data[20][rdIdx];
assign dataout[197:189] = data[21][rdIdx];
assign dataout[206:198] = data[22][rdIdx];
assign dataout[215:207] = data[23][rdIdx];
assign dataout[224:216] = data[24][rdIdx];
assign dataout[233:225] = data[25][rdIdx];
assign dataout[242:234] = data[26][rdIdx];
assign dataout[251:243] = data[27][rdIdx];
assign dataout[260:252] = data[28][rdIdx];
assign dataout[269:261] = data[29][rdIdx];
assign dataout[278:270] = data[30][rdIdx];
assign dataout[287:279] = data[31][rdIdx];
assign dataout[296:288] = data[32][rdIdx];

endmodule : perceptron
