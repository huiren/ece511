/*
`include "shfreg.sv"
`include "array.sv"
`include "predict.sv"
`include "sr.sv"
`include "v.sv"
*/
module fastpath_bp
(
    input clk,
    input [63:0] pc, last_pc,
    input make_prediction,
    input update,
    input br_outcome, last_prediction,
    input [31:0] last_h,
    input [32*6-1:0] last_v,
    input [32*8-1:0] last_r,
    output [31:0] h,
    output [32*6-1:0] sv_out,
    output [32*8-1:0] r,
    output prediction
     
);

parameter WEIGHT_WIDTH = 8;
parameter GHR_WIDTH = 32;
parameter WEIGHT_NUM = GHR_WIDTH + 1;
parameter WEIGHT_ENTRY_NUM = 64;


logic [WEIGHT_WIDTH*WEIGHT_NUM-1:0] sr_out, new_sr, w_out;
logic [GHR_WIDTH-1:0] sg_out, ghr_out;
logic pred;
logic [5:0] lookupIdx, updateIdx;
logic replace_sg;

assign lookupIdx = pc % WEIGHT_ENTRY_NUM;
assign updateIdx = last_pc % WEIGHT_ENTRY_NUM;

assign prediction = pred;
assign h = sg_out;
assign r = sr_out;

SR sr0
(
	.replace_in(last_r),
	.replace(replace_sg),
    .clk(clk),
    .din(new_sr),
    .load(make_prediction),
    .dout(sr_out)
);

SV sv0
(
	.clk(clk),
	.update(make_prediction),
	.lookupIdx(lookupIdx),
	.dout(sv_out)
);

neuron w0
(
    .clk(clk),
    .write(update),
    .last_h(last_h),
    .last_v(last_v),
    .br_outcome(br_outcome), 
    .last_prediction(last_prediction),
    .rdIdx(lookupIdx), 
    .wrIdx(updateIdx), 
    .dataout(w_out),
    .replace_sg(replace_sg)
);

shfreg sg0
(
    .clk(clk),
    .load(make_prediction),
    .in(pred),
    .replace_in(ghr_out),
    .replace(replace_sg),
    .out(sg_out)
);

shfreg ghr0
(
    .clk(clk),
    .load(update),
    .in(br_outcome),
    .out(ghr_out),
    .replace(0),
    .replace_in(0)
);


predict predict0
(
	.sr(sr_out),
	.w(w_out),
	.pred(pred),
	.new_sr(new_sr)
);

endmodule:fastpath_bp
