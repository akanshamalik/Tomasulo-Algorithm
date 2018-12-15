
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

// Latencies
#define ASCII_OFFSET 48
#define DELAY_ADD 1
#define DELAY_SUB 1
#define DELAY_MUL 9
#define DELAY_DIV 39


struct instruction
{
    uint8_t op;
    uint8_t dst;
    uint8_t srcOne;
    uint8_t srcTwo;
};

struct reservationStation
{
    bool busy;
    uint8_t op;
    int32_t vj;
    int32_t vk;
    uint8_t qj;
    uint8_t qk;
    bool disp;
};

struct integerAddUnit
{
    bool busy;
    int8_t cyclesRemaining;
    int32_t result;
    uint8_t dst; // rs dst 
    bool broadcast;
};

struct integerMultiplyUnit
{
    bool busy;
    int8_t cyclesRemaining;
    int32_t result;
    uint8_t dst; // rs dest 
    bool broadcast;
};

struct temporaryContainerForUpdate
{
	bool busy;
	uint8_t dst;
	int32_t result;
};

// Global Declarations
uint8_t numberOfInstructions;
uint32_t numberOfCycles;
int32_t registerFile[8]; // rf 
int8_t registerAllocationTable[8]; // rat
uint8_t instructionPosition = 0; // quea pointer
struct instruction instructions[10]; // insts
struct reservationStation rs[6]; //rs
struct integerAddUnit addUnit;
struct integerMultiplyUnit mulUnit;
struct temporaryContainerForUpdate temp;
char* strOpcodes[4] = { "Add", "Sub", "Mul", "Div" };
char* strTags[6] = { "", "RS1", "RS2", "RS3", "RS4", "RS5" }; // tags

// Function Declarations
void checkIssue( uint8_t instructionIndex );
void checkDispatch();
void checkBroadcast();
void printSimulatorOutput();
void checkUpdate();

/*
  Function: main
*/
int main( int argc, char * argv[] )
{
    printf( "\n Tomasulo's Algorithm Simulator by \n" ); 
     printf( "\n Nimish , Pranjal, Akansha \n" ); 
    // open inst file
	FILE * fptr;
        
		if( (fptr = fopen( "input.txt", "r" )) == NULL )
        {
            printf( "Input file error\n" );
            exit(1);
        }

        char line_buffer[BUFSIZ]; 
        uint8_t line_number = 0;
        line_number = 0;
        while( fgets(line_buffer, sizeof(line_buffer), fptr) )
        {
            ++line_number;
            if( line_number == 1 )
            {
                numberOfInstructions = line_buffer[0] - ASCII_OFFSET;
               
            }
            else if( line_number == 2 ) // no of cyles to execute 
            {
                numberOfCycles = atoi( line_buffer );
                
            }
            else if( line_number >= 3 && line_number < (3 + numberOfInstructions) ) // actual insts 
            {
                instructions[line_number-3].op = line_buffer[0] - ASCII_OFFSET;
                instructions[line_number-3].dst = line_buffer[2] - ASCII_OFFSET;
                instructions[line_number-3].srcOne = line_buffer[4] - ASCII_OFFSET;
                instructions[line_number-3].srcTwo = line_buffer[6] - ASCII_OFFSET;
            }
            else // rf values 
            {
                registerFile[line_number-3-numberOfInstructions] = atoi( line_buffer );
            }
        }

       

        // no of loops for cycles 
        for( uint16_t i = 0; i <= numberOfCycles; i++ )
        {
        	printf("\n\n\n");
        	printf("-----------------------------------------------------------------------\n");
        	printf("clock cycle %u \n\n",i);
        	printf("Reservation station");
        	printSimulatorOutput();
        	checkBroadcast();
			checkDispatch();
			checkIssue( instructionPosition );
			checkUpdate();
		}
    printf( "\n" );
    return 0;
}

/*
  Function: checkIssue
 */
void checkIssue( uint8_t instructionIndex )
{
    
    if( instructionIndex > numberOfInstructions - 1 )
    {
        return;
    }

    // Add and sub rs 
    if( instructions[instructionIndex].op == 0 || instructions[instructionIndex].op == 1 )
    {
        bool issuedSuccessfully = false;
        for ( uint8_t i = 1; i <= 3; i++) 
        {
            if( !rs[i].busy && !issuedSuccessfully )
            {
                rs[i].busy = true;
                rs[i].op = instructions[instructionIndex].op;

                
                if( registerAllocationTable[instructions[instructionIndex].srcOne] == 0 )
                {
                    rs[i].vj = registerFile[instructions[instructionIndex].srcOne];
                }
                else
                {
                    rs[i].qj = registerAllocationTable[instructions[instructionIndex].srcOne];
                }
                
                
                if( registerAllocationTable[instructions[instructionIndex].srcTwo] == 0 )
                {
                    rs[i].vk = registerFile[instructions[instructionIndex].srcTwo];
                }
                else
                {
                    rs[i].qk = registerAllocationTable[instructions[instructionIndex].srcTwo];
                }

                registerAllocationTable[instructions[instructionIndex].dst] = i; // update destination RAT with RS index
            
                issuedSuccessfully = true;
            }
        }

        if( issuedSuccessfully )
        {
            instructionPosition++; // move instruction queue to next instruction
        }
    }
    // mul and div rs 
    else if( instructions[instructionIndex].op == 2 || instructions[instructionIndex].op == 3 )
    {
        bool issuedSuccessfully = false;
        for ( uint8_t i = 4; i <= 5; i++) 
        {
            if( !rs[i].busy && !issuedSuccessfully )
            {
                rs[i].busy = true;
                rs[i].op = instructions[instructionIndex].op;

                
                if( registerAllocationTable[instructions[instructionIndex].srcOne] == 0 )
                {
                    rs[i].vj = registerFile[instructions[instructionIndex].srcOne];
                }
                else
                {
                    rs[i].qj = registerAllocationTable[instructions[instructionIndex].srcOne];
                }
                
                if( registerAllocationTable[instructions[instructionIndex].srcTwo] == 0 )
                {
                    rs[i].vk = registerFile[instructions[instructionIndex].srcTwo];
                }
                else
                {
                    rs[i].qk = registerAllocationTable[instructions[instructionIndex].srcTwo];
                }

                registerAllocationTable[instructions[instructionIndex].dst] = i; // update destination RAT with RS index
            
                issuedSuccessfully = true;
            }
        }

        if( issuedSuccessfully )
        {
            instructionPosition++; // move instruction queue to next instruction
        }
    }
    // opcode problem
    else
    {
        printf( "Incorrect instruction opcode: '%u'\n", instructions[instructionIndex].op );
        exit(EXIT_FAILURE);
    }
}

/*
   Function: checkDispatch
*/
void checkDispatch()
{
    if( rs[1].busy && !rs[1].disp )
    {
        if( addUnit.busy )
        {
            
        }
        else
        {
            if( rs[1].qj == 0 && rs[1].qk == 0 ) //if tags have captured their values
            {
                rs[1].disp = true;
                addUnit.busy = true;
                addUnit.dst = 1;
                switch( rs[1].op )
                {
                    case 0:
                        addUnit.result = rs[1].vj + rs[1].vk;
                        addUnit.cyclesRemaining = DELAY_ADD;
                        break;
                    case 1:
                        addUnit.result = rs[1].vj - rs[1].vk;
                        addUnit.cyclesRemaining = DELAY_SUB;
                        break;
                }
            }
            else
            {
               
            }
        }
    }

    if( rs[2].busy && !rs[2].disp )
    {
        if( addUnit.busy )
        {
            
        }
        else
        {
            if( rs[2].qj == 0 && rs[2].qk == 0 ) // if tags have captured their values
            {
                rs[2].disp = true;
                addUnit.busy = true;
                addUnit.dst = 2;
                switch( rs[2].op )
                {
                    case 0:
                        addUnit.result = rs[2].vj + rs[2].vk;
                        addUnit.cyclesRemaining = DELAY_ADD;
                        break;
                    case 1:
                        addUnit.result = rs[2].vj - rs[2].vk;
                        addUnit.cyclesRemaining = DELAY_SUB;
                        break;
                }
            }
            else
            {
                
            }
        }
    }

    if( rs[3].busy && !rs[3].disp )
    {
        if( addUnit.busy )
        {
            
        }
        else
        {
            if( rs[3].qj == 0 && rs[3].qk == 0 ) // if tags have captured their values
            {
                rs[3].disp = true;
                addUnit.busy = true;
                addUnit.dst = 3;
                switch( rs[3].op )
                {
                    case 0:
                        addUnit.result = rs[3].vj + rs[3].vk;
                        addUnit.cyclesRemaining = DELAY_ADD;
                        break;
                    case 1:
                        addUnit.result = rs[3].vj - rs[3].vk;
                        addUnit.cyclesRemaining = DELAY_SUB;
                        break;
                }
            }
            else
            {
                
            }
        }
    }

    if( rs[4].busy && !rs[4].disp )
    {
        if( mulUnit.busy )
        {
           
        }
        else
        {
            if( rs[4].qj == 0 && rs[4].qk == 0 ) //if tags have captured their values
            {
                rs[4].disp = true;
                mulUnit.busy = true;
                mulUnit.dst = 4;
                switch( rs[4].op )
                {
                    case 2:
                        mulUnit.result = rs[4].vj * rs[4].vk;
                        mulUnit.cyclesRemaining = DELAY_MUL;
                        break;
                    case 3:
                        mulUnit.result = rs[4].vj / rs[4].vk;
                        mulUnit.cyclesRemaining = DELAY_DIV;
                        break;
                }
            }
            else
            {
              
            }
        }
    }

    if( rs[5].busy && !rs[5].disp )
    {
        if( mulUnit.busy )
        {
            
        }
        else
        {
            if( rs[5].qj == 0 && rs[5].qk == 0 ) //if tags have captured their values
            {
                rs[5].disp = true;
                mulUnit.busy = true;
                mulUnit.dst = 5;
                switch( rs[5].op )
                {
                    case 2:
                        mulUnit.result = rs[5].vj * rs[5].vk;
                        mulUnit.cyclesRemaining = DELAY_MUL;
                        break;
                    case 3:
                        mulUnit.result = rs[5].vj / rs[5].vk;
                        mulUnit.cyclesRemaining = DELAY_DIV;
                        break;
                }
            }
            else
            {
             
            }
        }
    }
}

/*
   Function: checkBroadcast
 */
void checkBroadcast()
{
    bool broadcasting = false;
    if( mulUnit.busy )
    {
        if( mulUnit.cyclesRemaining == 0 ) // broadcast result
        {
            broadcasting = true;
	        mulUnit.busy = false;

			temp.busy = true;
			temp.dst = mulUnit.dst;
			temp.result = mulUnit.result;
        }

        if( mulUnit.cyclesRemaining > 0 )
        {
            mulUnit.cyclesRemaining--; // decrement cycles remaining
        }
    }

    if( addUnit.busy ) // broadcast ADD unit result 
    {   
        if( addUnit.cyclesRemaining == 0 && !broadcasting ) // broadcast ADD unit result
        {
	        addUnit.busy = false;

			temp.busy = true;
			temp.dst = addUnit.dst;
			temp.result = addUnit.result;
            
        }
        else
        {
            if( broadcasting )
            {
                
            }
        }

        if( addUnit.cyclesRemaining > 0 )
        {
            addUnit.cyclesRemaining--; // decrement cycles remaining
        }
    }
}

/*
   Function: checkUpdate
  */
void checkUpdate()
{
	if( temp.busy )
	{
		// update matching reservation station values (vj, vk) and clear tags (qj, qk)
		for( uint8_t i = 1; i <= 5; i++ ) 
        {
            if( rs[i].qj == temp.dst )
            {
                rs[i].qj = 0;
                rs[i].vj = temp.result;
            }
            
            if( rs[i].qk == temp.dst )
            {
                rs[i].qk = 0;
                rs[i].vk = temp.result;
            }
        }
		
		// clear matching RAT tags and update RF values
        for( uint8_t i = 0; i < 8; i++ )
        {
            if( registerAllocationTable[i] == temp.dst )
            {
                registerAllocationTable[i] = 0;
                registerFile[i] = temp.result;
            }
        }
		
		// clear reservation station
        rs[temp.dst].busy = false;
	    rs[temp.dst].disp = false;
	    rs[temp.dst].op = rs[temp.dst].vj = rs[temp.dst].vk = rs[temp.dst].qj = rs[temp.dst].qk = 0;
		
		// clear temporaryContainer
		temp.busy = false;
		temp.dst = temp.result = 0;
	}
}

/*
  Function: printSimulatorOutput
 */
void printSimulatorOutput()
{
	
	printf( "\n\tBusy\tOp\tVj\tVk\tQj\tQk\tDisp\n" );
	
	// print values
	for( uint8_t i = 1; i <= 5; i++ )
	{
		printf( "RS%u\t%u\t", i, rs[i].busy );
        if( rs[i].busy )
        {
            printf( "%s\t", strOpcodes[rs[i].op] );

            if( rs[i].qj == 0 ) 
            {
                printf( "%d\t", rs[i].vj );
            }
            else
            {
                printf( " \t" );
            }

            if( rs[i].qk == 0 ) 
            {
                printf( "%d\t", rs[i].vk );
            }
            else
            {
                printf( " \t" );
            }

            printf( "%s\t%s\t%u\n", strTags[rs[i].qj], strTags[rs[i].qk], rs[i].disp );
        }
        else
        {
            printf( " \t \t \t \t \t \n" );
        }
    }
	
	printf( "\n\tRF\t\tRAT\n" );
	for( uint8_t i = 0; i <= 7; i++ )
	{
		printf( "%u:\t%d", i, registerFile[i] );
		
		if( registerAllocationTable[i] != 0 )
		{
			printf( "\t\t%s\n", strTags[registerAllocationTable[i]] );
		}
		else
		{
			printf( "\t\t \n" );
		}
	}
	printf( "\nInstruction Queue\n" );
	for( uint8_t i = instructionPosition; i < numberOfInstructions; i++ )
	{
		printf( "%s R%u, R%u, R%u\n", strOpcodes[instructions[i].op], instructions[i].dst, instructions[i].srcOne, instructions[i].srcTwo );
	}
}


