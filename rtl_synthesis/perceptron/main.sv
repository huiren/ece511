`include "register.sv"
`include "shfreg.sv"
`include "array.sv"

//`define WEIGHT_NUM 33
//`define WEIGHT_ENTRY_NUM 4096

module perceptron_bp (
	input clk,    // Clock
	input [63:0] pc,
	//input lookup, // Activate when last BR is resolved
	input update, // Activate when fetching a new BR
	input br_outcome, // Resolved BR
	output logic pred // Prediction

);

parameter GHR_WIDTH = 32;
parameter WEIGHT_NUM = GHR_WIDTH + 1;
parameter WEIGHT_ENTRY_NUM = 4096;

logic [11:0] lookupIdx, updateIdx;
logic [GHR_WIDTH-1:0] ghr_out;
logic [WEIGHT_NUM*9-1:0] perceptrons_out;

assign lookupIdx = pc % 4096;
assign updateIdx = pc % 4096;

shfreg ghr0 (
    .clk(clk),
    .load(update),
    .in(br_outcome),
    .out(ghr_out)
);

perceptron p0 (
	.clk(clk),
	.write(update),
	.ghr(ghr_out),
	.br_outcome(br_outcome),
	.rdIdx(lookupIdx),
	.wrIdx(updateIdx),
	.dataout(perceptrons_out)
);

predict predict0 (
	.ghr(ghr_out),
	.p(perceptrons_out),
	.pred(pred)
);

endmodule : perceptron_bp
