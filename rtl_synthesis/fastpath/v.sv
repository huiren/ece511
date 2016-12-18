module SV #(parameter WIDTH = 6, parameter SIZE = 32)
(
    input clk,
    input update,
    input [5:0] lookupIdx,
    output [WIDTH*SIZE-1:0] dout
);

logic [WIDTH*SIZE-1:0] data;

assign dout = data;

always_ff @ (posedge clk)
begin
    if (update)
        data <= {lookupIdx, data[WIDTH*SIZE-1:WIDTH]};
end

endmodule: SV

