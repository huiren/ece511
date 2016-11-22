module SR #(parameter WIDTH = 8, parameter SIZE = 33)
(
	input [WIDTH*SIZE-1:0] replace_in,
	input replace,
    input clk,
    input [WIDTH*SIZE-1:0] din, //summed new SR
    input load,
    output [WIDTH*SIZE-1:0] dout,
);

logic [WIDTH*SIZE-1:0] data;

assign dout = data;

always_ff @ (posedge clk)
begin
	if (replace_in)
		data <= replace_in;
    else if (load)
        data <= din;
end

endmodule: SR

