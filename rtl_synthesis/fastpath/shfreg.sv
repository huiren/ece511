module shfreg #(parameter width = 32)
(
    input clk,
    input [width-1:0] replace_in,
    input load, replace,
    input in,
    output logic [width-1:0] out
);

logic [width-1:0] data;

/* Altera device registers are 0 at power on. Specify this
 * so that Modelsim works as expected.
 */


always_ff @(posedge clk)
begin
	if (replace)
		data <= replace_in;
    if (load)
        data <= {in, data >> 1};
end

assign out = data;

endmodule : shfreg

