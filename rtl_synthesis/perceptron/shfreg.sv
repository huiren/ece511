module shfreg #(parameter width = 32)
(
    input clk,
    input load,
    input in,
    output logic [width-1:0] out
);

logic [width-1:0] data;

/* Altera device registers are 0 at power on. Specify this
 * so that Modelsim works as expected.
 */

always_ff @(posedge clk)
begin
    if (load)
    begin
        data = {in, data >> 1};
    end
end

assign out = data;

endmodule : shfreg

