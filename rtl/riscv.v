


module riscv_cpu( 
    input sys_clk,          // clk input
    input sys_rst_n,        // reset input
    output reg [5:0] led,   // 6 LEDS pin
    output			LCD_CLK,
	output			LCD_HYNC,
	output			LCD_SYNC,
	output			LCD_DEN,
	output	[4:0]	LCD_R,
	output	[5:0]	LCD_G,
	output	[4:0]	LCD_B
	
);

 
wire[31:0] romline;
reg[31:0] registers[0:31];//cpu registers
reg [31:0] pc=0;//program counter
reg [6:0] opcode;
wire[11:0] imm;  
wire[11:0] imms; 
wire[11:0] immc; 
wire[11:0] immlui;
wire[4:0] rd;
reg[4:0] rd_load=0;
wire[4:0] rs1;
wire[4:0] rs2;

wire[19:0] imm20;
wire wr_enable;
wire[31:0] address;
wire[2:0] func3;
wire[6:0] func7;

reg[31:0] datwr=0;

reg[10:0] store=11'b0;
reg[10:0] load=0;
reg[31:0] storeloadaddr=0;

integer i;
wire oce =1;
wire ce =1;
wire reset =0;
reg store_done=0;
 
reg [4:0] red =  8'b00000000;
reg [5:0] green =8'b00000000;
reg [4:0] blue = 8'b00000000;

assign LCD_R = red;

assign LCD_B = blue;
assign LCD_G = green;

reg [20:0] CounterX;  // counts from 0 to 799
reg [20:0] CounterY;  // counts from 0 to 524
reg [12:0] xcnt =0;
reg [12:0] ycnt =0;
reg [12:0] xcor =0;
reg [12:0] ycor =0;


//reg [2047:0] screen[0:23]; //this memory and acreen memory in ram must be synced/cloned
reg [2047:0] screen ={2048{1'b0}}; //this memory and acreen memory in ram must be synced/cloned

wire cpu_clk;
wire clk21;



assign rd=romline[11:7];
 
assign address=load || store ? storeloadaddr >>2 : pc>>2;
assign rs1=romline[19:15];
assign rs2=romline[24:20];
assign imms={romline[31:25],romline[11:7]};
assign immc={romline[31],romline[7],romline[30:25],romline[11:8]};
assign imm=romline[31:20];
assign imm20={romline[31],romline[19:12],romline[20],romline[30:21]};
assign immlui=romline[31:12];
assign func3=romline[14:12];
assign func7=romline[31:25];
assign wr_enable=store>0;

 
//cursor start from 200X200
	

Gowin_SP ram(
    .dout(romline), //output [31:0] dout
    .clk(clk21), //input clk
    .oce(oce), //input oce
    .ce(ce), //input ce
    .reset(reset), //input reset
    .wre(wr_enable), //input wre
    .ad(address), //input [12:0] ad
    .din(datwr) //input [31:0] din
);







Gowin_rPLL lcd_clk(
        .clkout(LCD_CLK), //output clkout
        .clkin(sys_clk) //input clkin
    );
    Gowin_CLKDIV divided2(
        .clkout(cpu_clk), //output clkout
        .hclkin(clk21), //input hclkin
        .resetn(sys_rst_n) //input resetn
    );


    Gowin_rPLL2  mhz21gen(
        .clkout(clk21), //output clkout
        .clkin(sys_clk) //input clkin
    );
wire[9:0] vertc;
wire[9:0] horc; 
vga	vgaa(
		.clk (LCD_CLK),
		.vsync(LCD_SYNC),
        .hsync(LCD_HYNC),
        .vertc(vertc),
        .horc(horc),
        .de(LCD_DEN)

	
	);




integer counter=0;
always @(posedge LCD_CLK)begin
   
if(LCD_SYNC)begin
    ycor<=0;
    xcor<=0;
    xcnt <=0;
    ycnt<=0;
end
    

   if(LCD_DEN)begin
                
                 if(horc>=16+43 && horc<(448+43)+16 && vertc>=12+8 && vertc<(256+12)+8)
                  begin
//               
                if(xcnt<6)xcnt<=xcnt+1;
                       else begin

                            xcnt <=0;

                            if(xcor <63)xcor<=xcor+1;
                                else begin
                                    xcor<=0;
                                    if(ycnt<7)ycnt<=ycnt+1;
                                    else begin
                                        ycor<=ycor+1;
                                        
                                        ycnt<=0;
                                    end

                                end
                            end

                green<={8{screen[ ((ycor<<6)+xcor)]}};
//                if(horc == 16+43 && vertc == 12+8)green<=8'b11111111;
//                else  if(horc == (448+43)+16 && vertc ==12+8)green<=8'b11111111;
//                else  if(horc == 16+43 && vertc == (256+12)+8)green<=8'b11111111;
//                else   if(horc == (448+43)+16 && vertc == (256+12)+8)green<=8'b11111111;
//                else green<=8'b0;

              
               blue <=8'b0;
               red <= 8'b0;
           end
          else begin
                    green<=8'b0;
                    blue <=8'b0;
                    red <= 8'b0;
                     
                    
          end

    end
   else begin
     green<=1'b0;
     
      end  

    blue <=8'b0;
    red <= 8'b0;
                     
                
                    
end


//riscv core logic
always @(posedge cpu_clk)
begin
     
     if(load)begin
                 
                case(load)
             
                        8:
                            case(storeloadaddr-(address<<2))
                                0:registers[rd_load]<= {{24{romline[7]}},romline[7:0]};
                                1:registers[rd_load]<= {{24{romline[15]}},romline[15:8]};
                                2:registers[rd_load]<= {{24{romline[23]}},romline[23:16]};
                                3:registers[rd_load]<= {{24{romline[31]}},romline[31:24]};
                            endcase
                        16:
                                    
                            case(storeloadaddr-(address<<2))
                                0:registers[rd_load]<= {{16{romline[15]}},romline[15:0]};
                                2:registers[rd_load]<= {{16{romline[31]}},romline[31:16]};
                            endcase

                        32:registers[rd_load]<=romline;
                        81:
                            case(storeloadaddr-(address<<2))
                                0:registers[rd_load]<= romline[7:0];
                                1:registers[rd_load]<= romline[15:8];
                                2:registers[rd_load]<= romline[23:16];
                                3:registers[rd_load]<= romline[31:24];
                            endcase
                        161:
             
                            case(storeloadaddr-(address<<2))
                                0:registers[rd_load]<= romline[15:0];
                                2:registers[rd_load]<= romline[31:16];
                            endcase
                        default:i<=0;
                             
                endcase
                load<=0;
                pc<=pc+4;
    end
    //if previous instruction was store data in memory
    else if(store>0)begin
        
        store_done<=1;
        if(store_done==0)begin
            case(store)
                8: case(storeloadaddr-(address<<2))
                        0:datwr<={romline[31:8],datwr[7:0]};
                        1:datwr<={romline[31:16],datwr[7:0],romline[7:0]};
                        2:datwr<={romline[31:24],datwr[7:0],romline[15:0]};
                        3:datwr<={datwr[7:0],romline[23:0]};
                    endcase
                16:
                    case(storeloadaddr-(address<<2))
                        0:datwr<={romline[31:16],datwr[15:0]};
                        2:datwr<={datwr[15:0],romline[15:0]};
                     endcase

                32:datwr<=datwr;
                default:datwr<=0;


            endcase
            //LEDS address in rom
            if(address == (7504))led<=datwr;

            //this is screen adress on c rom
            else if(address >=(7508))
                screen[(address-(7508))]<=datwr&1;


        end
        else begin
            store<=0;
            store_done<=0;
            pc<=pc+4;
        end

        end


    else begin
    //fetch opcode  if not store or load
 
        case(romline[6:0])          
           //Integer Register-Immediate Instructions
            7'b0010011:begin
                     case(func3)
                            //ADDI
                            3'b000:registers[rd]<={{20{imm[11]}},imm}+registers[rs1];
                            //SLTI
                          3'b010:registers[rd]<=$signed(registers[rs1])<$signed({{20{imm[11]}},imm});
                           //SLTIU
                            3'b011:registers[rd]<=registers[rs1]<{{20{imm[11]}},imm};
                            //XORI
                            3'b100:registers[rd]<=registers[rs1] ^ {{20{imm[11]}},imm} ;      		 
                          //ORI
                            3'b110:registers[rd]<=registers[rs1] | {{20{imm[11]}},imm} ;
                           //ANDI
                            3'b111:registers[rd]<=registers[rs1] & {{20{imm[11]}},imm} ;
                            //SLLI
                            3'b001:registers[rd]<=registers[rs1] << imm[4:0] ;
                           //SRLI,SRAI
                            3'b101:
                                case(imm[11:7])
                                     //SRLI
                                     7'b0000000:registers[rd]<=registers[rs1] >> imm[4:0] ;
                                     //SRAI
                                     7'b0001000:registers[rd]<=registers[rs1] >>> imm[4:0] ;
                                     default:i<=0;
                                endcase
                     default:i<=0;
                     endcase
                     pc<=pc+4;
                
                 end
            //LUI
            7'b0110111:begin
                   
                 registers[rd]<=immlui<<12;
                 pc<=pc+4;
                 
            end
            
            //AUIPC
            7'b0010111:begin
                 
                registers[rd]<=pc+(immlui<<12);
                pc<=pc+4;
            
            end
            
            //Integer Register-Register Operations
            7'b0110011:begin
             
                case(func3)
                    
                    //ADD,SUB
                    3'b000:
                          case(func7)	
                                7'b0000000:registers[rd]<=registers[rs1] + registers[rs2]  ;
                                7'b0100000:registers[rd]<=registers[rs1] - registers[rs2]  ;	
                            default:i<=0;					
                          endcase
                    
                    //SLL
                    3'b001:registers[rd]<=registers[rs1] << registers[rs2]  ;
                    //SLT
                    3'b010:registers[rd]<=$signed(registers[rs1]) < $signed(registers[rs2]);
                    //SLTU
                    3'b011:registers[rd]<=registers[rs1] < registers[rs2]  ;
                    //XOR
                    3'b100:registers[rd]<=registers[rs1] ^ registers[rs2]  ;
                    //SRL,SRA 
                    3'b101:
                         case(func7)
                    
                            //SRL
                            7'b0000000:registers[rd]<=registers[rs1] >> registers[rs2]  ;
                            //SRA
                            7'b0100000:registers[rd]<=registers[rs1] >>> registers[rs2]  ;
                         default:i<=0;
                        endcase	
                    //OR
                    3'b110:registers[rd]<=registers[rs1] | registers[rs2]  ;
                     //AND
                    3'b111:registers[rd]<=registers[rs1] & registers[rs2]  ;
                        default:i<=0;
                    
                endcase
                
                 pc<=pc+4;
                end
                

            
            
            
            

           //JAL
            7'b1101111:begin
                   if(rd)
                      registers[rd]<=pc+4;
                    pc<=pc+({{11{imm20[19]}},imm20,1'b0});	  
                    
                end
           //JALR
            7'b1100111:begin
                
                 if(rd)
                      registers[rd]<=pc+4;
                 pc<=($signed(registers[rs1])+$signed({{20{imm[11]}},imm})) & 'hfffffffe;	
                
                end
              

              
              
           //Load Instructions   
            7'b0000011:begin
                     rd_load<=romline[11:7];
                     storeloadaddr<={{{20{imm[11]}}},imm}+registers[rs1];
                     
                     case(func3)
                         //LB
                         3'b000:load<=8;
                         //LH		 
                         3'b001:load<=16;
                         //LW
                         3'b010:load<=32;
                         //LBU
                         3'b100:load<=81;
                         //LHU
                         3'b101:load<=161;		  	 
                         default:i<=0;
                     endcase
                end
                 
                 
           //Store Instructions
           7'b0100011:begin
                 
                        storeloadaddr<={{{20{imms[11]}}},imms}+registers[rs1];
                     
                        case(func3)
                         //SB
                         3'b000:begin
                                   datwr<=registers[rs2][7:0];
                                   store<=8;
                               end
                         //SH
                         3'b001:begin
                                   datwr<=registers[rs2][15:0];
                                   store<=16;
                               end
                         //SW
                         3'b010:begin
                                   datwr<=registers[rs2];
                                   store<=32;
                                end
                        default:i<=0;
                        endcase
                end
           
            
            
          //Conditional Branches   
            7'b1100011:begin 
                case(func3)
                 
                
                 //BGE
                 3'b101: 
                            if($signed(registers[rs1])>=$signed(registers[rs2]))
                                 pc<=pc+{{19{immc[11]}},immc,1'b0};
                            else pc<=pc+4;
                //BEQ
                 3'b000:
                            if(registers[rs1]==registers[rs2])
                                pc<=pc+{{19{immc[11]}},immc,1'b0};
                            else pc<=pc+4;
                     
                 //BNE
                 3'b001:
                            if(registers[rs1]!=registers[rs2])
                                        pc<=pc+{{19{immc[11]}},immc,1'b0};
                            else pc<=pc+4;
                 
                 
                 //BLT
                 3'b100:
                            if($signed(registers[rs1])<$signed(registers[rs2]))
                                         pc<=pc+{{19{immc[11]}},immc,1'b0};
                            else pc<=pc+4;
                 
                 //BLTU
                 3'b110:
                            if(registers[rs1]<registers[rs2])
                                        pc<=pc+{{19{immc[11]}},immc,1'b0};
                                else pc<=pc+4;
                 
                 //BGEU
                 3'b111:
                            if(registers[rs1]>=registers[rs2])
                                        pc<=pc+{{19{immc[11]}},immc,1'b0};
                            else pc<=pc+4;
                 
                 
                 
                 
                default:i<=0;
                 endcase
                 
                 
                             
                 end 
                 


        default:i<=0;
        endcase
    end


end
 
endmodule
