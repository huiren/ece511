module perceptron #(parameter WEIGHT_NUM = 17, parameter WEIGHT_ENTRY_NUM = 32)
(
    input clk,
    input write, // 
    input [15:0] ghr,
    input br_outcome,
    input [4:0] rdIdx, wrIdx, // Max 4096
    output logic [WEIGHT_NUM*8-1:0] dataout // 33 * 9 = 297
);

// 9-bit per weight, 33 weights per branch, 4096 branch records
logic[7:0] data [WEIGHT_NUM][WEIGHT_ENTRY_NUM];



/* Initialize array */
/*
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
*/

always_ff @(posedge clk)
begin
	// Self update involved
    if (write == 1)
    begin
        if (br_outcome == 1)
            data[0][wrIdx] <= data[0][wrIdx] + 8'd1;
        else
            data[0][wrIdx] <= data[0][wrIdx] - 8'd1;
            
    	for (int k = 1; k < WEIGHT_NUM; k++)
    	begin
    		data[k][wrIdx] <= (br_outcome == ghr[k-1] ? data[k][wrIdx] + 8'd1: data[k][wrIdx] - 8'd1);
    	end
    end 
end


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
endmodule : perceptron
