module SR #(parameter WIDTH = 8, parameter SIZE = 33)
(
    input clk,
    input [WIDTH*SIZE-1:0] din, //summed new SR
    input load,
    output [WIDTH*SIZE-1:0] dout,
)

logic [WIDTH*SIZE-1:0] data;

assign dout = data;

always_ff @ (posedge clk)
begin
    if (load)
        data <= din;
end

endmodule: SR

