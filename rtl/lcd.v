module vga(
input clk,

//output	[5:0] g,//vga rgb output
output vsync,//vga vsync output
output hsync,////vga hsync output
output de,
output reg[9:0] vertc=0,
output reg[9:0] horc=0
);





//vga timings get from http://tinyvga.com/vga-timing/640x480@60Hz

//vsync must be 1 in "Sync pulse"	clock tange in vertical timings 
assign hsync = horc>= (43+480+4) && horc <= (43+480 + 4 + 4);

//hsync must be 1 in Sync pulse	clock tange in hotizontal timings
assign vsync = vertc >= (12+272 + 4) && vertc <= (12+272 + 4+4);

assign de = (horc >= 43 && horc  <= (43+480) && vertc >= 12 && vertc <= (12+272)) ? 1'b1:1'b0;


//pixel data must be writen when hitozontal and vertical clock counter is in that range(0,640),(0,480)
//if not in range, data must be blank
//3'b010 that data fill all screen to green
//assign g=(horc > 43 && horc <= (43+480) && vertc > 12 && vertc <= (12+276))?6'b111111:1'b0; 
//assign g=(horc > 43 && horc <= (43+280) &&
//         vertc > 12 && vertc <= (12+176)) ?6'b111111:1'b0; 






always @(posedge clk) begin
      //clock counter for vertical and horizontal sync
		if (horc<530) begin
			 horc<=horc+1;
			end
		else begin
			  horc<=0;
			  if(vertc<291) begin
					vertc<=vertc+1;
				  end
			  else begin
					vertc<=0;
				    end   	
			 end	 
				
end


endmodule

