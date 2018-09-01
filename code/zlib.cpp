//TODO(Rename to region.cpp)
int symbols_decoded[0x8000]={};
int symbols_decoded_place=0;
umo global_start_buffer = 0;
u8* out_arr=0;
static u32 read_byte(void** buffer)
{
	u32 byte=((u8*)buffer[0])[0];
	(*(umo*)buffer)+=1;
	return byte;
}
static u32 peek_byte(void** buffer)
{
	u32 byte=((u8*)buffer[0])[0];
	return byte;
}
static u64 read_n_bytes_BE(void** buffer,int n)
{
	u64 value=0;
	forei(n)
	{
		value=value*0x100+read_byte(buffer);
	}
	return value;
}
static void sort_static_array(int* array,int* sorted_array,int length)
{
	forei(1024)
	{
		sorted_array[i]=array[i];
	}
	forei(1024)
	{
		for(int j=i+1;j<1024;j++)
		{
			if(sorted_array[i]>sorted_array[j])
			{
				int temp=sorted_array[j];
				sorted_array[j]=sorted_array[i];
				sorted_array[i]=temp;
			}
		}
	}
}
static void fix_buffer_and_offset(void** buffer,int* offset_p)
{
	int offset=*offset_p;
	while(offset>=8)
	{
		*buffer=(void*)((*(umo*)buffer)+1);
		offset-=8;
	}
	*offset_p=offset;
}
static u32 read_n_bits(void** buffer,int* offset_p,int n)
{
	int offset=*offset_p;
	u32 value=0;

	u32 current_byte = peek_byte(buffer);
	value=(((*(u32*)*buffer)>>offset)&((1u<<n)-1u));
	offset+=n;
	*offset_p = offset;
	fix_buffer_and_offset(buffer,offset_p);
	return value;
}
static u16 peek_15_bit_offset(void** buffer,int* offset_p)
{
	int offset=*offset_p;
	u16 value=0;

	value=(u16)(((*(u32*)*buffer)>>offset)&((1<<15u)-1));
	return value;
}

static u8 peek_7_bit_offset(void** buffer,int* offset_p)
{
	int offset=*offset_p;
	u8 value=0;

	value=(u8)(((*(u32*)*buffer)>>offset)&((1<<7u)-1));
	return value;
}
__declspec(noinline) 
static u16 update_word_by_offset(void** buffer,int* offset_p,u32 change)
{
	*offset_p+=change;
	fix_buffer_and_offset(buffer,offset_p);
	u16 value= peek_15_bit_offset(buffer,offset_p);
	return value;
}
static u16 reverse_u16(u16 symbol,u32 length)
{
	u16 result=0;
	for(u32 i=0;i<length;i++)
	{
		result=(result<<1)+(symbol&1u);
		symbol>>=1;
	}
	return result;
}
static void turn_lengths_into_symbols(int* symbols, u32* lengths,int n)
{
	int length_sum[16]={};
	int next_symbol[16]={};
	forei(n)
	{
		Assert(lengths[i]<16);
		length_sum[lengths[i]]++;
	}
	int current_code=0;
	int check_for_full_tree=1;
	length_sum[0] = 0;
	for(int i=1;i<16;i++)
	{
		check_for_full_tree*=2;
		check_for_full_tree-=length_sum[i];
		current_code=(current_code+length_sum[i-1])<<1;
		next_symbol[i]=(u16)current_code;
	}
	Assert(check_for_full_tree==0);
	forei(n)
	{
		u32 bits=lengths[i];
		if(bits)
		{
			symbols[i]=next_symbol[bits]++;
		}
	}
}
static void generate_LUT_from_symbols(u32* sym_LUT, u32* len_LUT, int lut_len, int* symbols, u32* sym_lens,int sym_num)
{
	for(int i=0;i<sym_num;i++)
	{
				u32 bits=sym_lens[i];
				if(bits!=0)
					symbols[i]=reverse_u16((u16)symbols[i],bits);
	}
	for(int i=0;i<lut_len;i++)
	{
		if(len_LUT[i]==0)
		{
			for(int j=sym_num-1;j>=0;j--)
			{
				u32 bits=sym_lens[j];
				if(bits!=0)
				{
					int mask=(1<<bits)-1;
					// u16 i_reversed=reverse_u16((u16)i,bits);
					if(((i^symbols[j])&(mask))==0)
					{
						for(int i_tag=i;i_tag<lut_len;i_tag+=(1<<(bits)))//maybe it should be bits
						{
							Assert(len_LUT[i_tag] == 0);
							len_LUT[i_tag]=(u16)bits;
							sym_LUT[i_tag]=(u16)j;
						}
					}
				}
			}
			Assert(len_LUT[i]!=0);
		}
	}
}
static void	decode_code_length_huffman(void** buffer,int* offset,u32* literal_sym_LUT,u32* literal_len_LUT,u32* dist_sym_LUT,u32* dist_len_LUT,u32* max_code)
{
	umo buffer_start=(umo)*buffer;
	u32 code_length_length[19]={};
	int code_length_symbols_reversed[19]={};
	int literal_symbols[286]={};
	int dist_symbols[33]={};
	u32 code_length_symbol_lut[128]={};
	u32 code_length_length_lut[128]={};
	u32 code_length_buffer[286+33]={};
	int HLIT=257+(int)read_n_bits(buffer,offset,5);
	int HDIST=1+(int)read_n_bits(buffer,offset,5);
	int HCLEN=4+(int)read_n_bits(buffer,offset,4);
	//code_length_length order is:
	int code_length_length_order[19]={16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
	if(1)
		for(int i=0;i<HCLEN;i++)
	{
		code_length_length[code_length_length_order[i]]=read_n_bits(buffer,offset,3);
	}
	else if(0)
	{
		code_length_length[0]=1;
		code_length_length[1]=2;
		code_length_length[2]=3;
		code_length_length[3]=3;
	}
	else
	{
		code_length_length[0]=2;
		code_length_length[1]=3;
		code_length_length[2]=3;
		code_length_length[3]=3;
		code_length_length[4]=3;
		code_length_length[5]=3;
		code_length_length[6]=4;
		code_length_length[7]=4;
	}
	turn_lengths_into_symbols(code_length_symbols_reversed,code_length_length,19);
	generate_LUT_from_symbols(code_length_symbol_lut,code_length_length_lut,1<<7,code_length_symbols_reversed,code_length_length,19);
	
	u8 B0=(u8)peek_7_bit_offset(buffer,offset);
	int place=0;
	*max_code=0;
	while(place<HLIT+HDIST)
	{
		u32 symbol_length=code_length_length_lut[B0];
		u32 symbol=code_length_symbol_lut[B0];
		Assert(symbol_length>0);
		*offset+=symbol_length;
		fix_buffer_and_offset(buffer,offset);
		B0=(u8)peek_7_bit_offset(buffer,offset);
		int copy_length=0;
		if(symbol>15)
		{
			int repeat=0;
			u32 new_symbol=0;
			if(symbol==16)
			{
				copy_length=2;
				repeat=3+(B0&3);
				new_symbol=code_length_buffer[place-1];
			}
			if(symbol==17)
			{
				copy_length=3;
				repeat=3+(B0&7);
			}
			if(symbol==18)
			{
				copy_length=7;
				repeat=11+(B0&127);
			}

			for(int i=0;i<repeat;i++)
			{
				code_length_buffer[place++]=new_symbol;
			}
			*offset+=copy_length;
			fix_buffer_and_offset(buffer,offset);
			B0=(u8)peek_7_bit_offset(buffer,offset);
		}
		else
		{
			code_length_buffer[place++]=symbol;
			if(*max_code<symbol) *max_code=symbol;
		}
	}
	Assert(place==HDIST+HLIT);

	turn_lengths_into_symbols(literal_symbols,code_length_buffer,HLIT);
	generate_LUT_from_symbols(literal_sym_LUT,literal_len_LUT,1<<(*max_code),literal_symbols,code_length_buffer,HLIT);
	turn_lengths_into_symbols(dist_symbols,code_length_buffer+HLIT,HDIST);
	generate_LUT_from_symbols(dist_sym_LUT,dist_len_LUT,1<<(*max_code),dist_symbols,code_length_buffer+HLIT,HDIST);
}
static void read_and_parse_zlib_buffer(void* buffer, umo buffer_length)
{
    /*len lut
			 257   0     3       267   1   15,16     277   4   67-82
             258   0     4       268   1   17,18     278   4   83-98
             259   0     5       269   2   19-22     279   4   99-114
             260   0     6       270   2   23-26     280   4  115-130
             261   0     7       271   2   27-30     281   5  131-162
             262   0     8       272   2   31-34     282   5  163-194
             263   0     9       273   3   35-42     283   5  195-226
             264   0    10       274   3   43-50     284   5  227-257
             265   1  11,12      275   3   51-58     285   0    258
             266   1  13,14      276   3   59-66
	 dist lut  
			  ---- ---- ----  ---- ----  ------    ---- ---- --------
               0   0    1     10   4     33-48    20    9   1025-1536
               1   0    2     11   4     49-64    21    9   1537-2048
               2   0    3     12   5     65-96    22   10   2049-3072
               3   0    4     13   5     97-128   23   10   3073-4096
               4   1   5,6    14   6    129-192   24   11   4097-6144
               5   1   7,8    15   6    193-256   25   11   6145-8192
               6   2   9-12   16   7    257-384   26   12  8193-12288
               7   2  13-16   17   7    385-512   27   12 12289-16384
               8   3  17-24   18   8    513-768   28   13 16385-24576
               9   3  25-32   19   8   769-1024   29   13 24577-32768
*/
	void* start_buffer = buffer;
	global_start_buffer=(umo)start_buffer;
	u32 start_len_lut[]={3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258};
	u32 extra_bits_len_lut[]={0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};
	u32 start_dist_lut[]={1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577};
	u32 extra_bits_dist_lut[]={0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};
	u32 CMF=read_byte(&buffer);
	u32 compression = CMF & 15;
	u32	CINFO = (CMF >> 4) & 15;
	int window_length=1<<(CINFO+8);
	u32 FLG = read_byte(&buffer);
	
	Assert(compression==8);
	Assert((CMF * 0x100 + FLG) % 31 == 0);
	u32 dict=FLG&(1<<5);
	u32 FLEVEL=FLG>>6;
	if (dict)
	{
		read_n_bytes_BE(&buffer,4);
	}
	int offset=0;
	umo place=0;
	u32 BFINAL;
	do
	{
		BFINAL = read_n_bits(&buffer, &offset, 1);
		u32 BTYPE = read_n_bits(&buffer, &offset, 2);
		if(BTYPE==0)
		{
			offset=0;
			*(umo*)&buffer+=1;
			u64 LEN=read_n_bytes_BE(&buffer,2);
			u64 NLEN=read_n_bytes_BE(&buffer,2);
			Assert(((u16)(LEN+NLEN))==0);
			es_memcopy_bytes(out_arr,buffer,(int)LEN);
			place+=LEN;
			*(umo*)&buffer+=LEN;
		}
		if(BTYPE==2)
		{

			u32 lit_sym_LUT[1<<15]={};
			u32 lit_len_LUT[1<<15]={};
			u32 dist_sym_LUT[1<<15]={};
			u32 dist_len_LUT[1<<15]={};
			u32 max_code;
			decode_code_length_huffman(&buffer,&offset,lit_sym_LUT,lit_len_LUT,dist_sym_LUT,dist_len_LUT,&max_code);
			fix_buffer_and_offset(&buffer,&offset);
			u16 W0=peek_15_bit_offset(&buffer,&offset);
			int symbol_place = 0;
			int mask=((1<<max_code)-1);
			while(true)
			{
				u32 symbol=lit_sym_LUT[W0&mask];
				u32 symlen=lit_len_LUT[W0&mask];
				// if (symbol!=symbols_decoded[symbol_place])
				// {
				// 	DebugBreak();
				// }
				umo pos=(umo)(buffer)-global_start_buffer;
				symbol_place++;
				Assert((umo)buffer<(umo)start_buffer+buffer_length)
				W0=update_word_by_offset(&buffer,&offset,symlen);
				if(symbol==256)
				{
					break;
				}
				if(symbol>=257)
				{
					u32 length=start_len_lut[symbol-257];
					u32 extra_bits_length=extra_bits_len_lut[symbol-257];
					if(extra_bits_length)
					{
						length+=W0&((1<<extra_bits_length)-1);
						W0=update_word_by_offset(&buffer,&offset,extra_bits_length);
					}
					u32 dist_code=dist_sym_LUT[W0&mask];
					u32 dist_code_len=dist_len_LUT[W0&mask];
					W0=update_word_by_offset(&buffer,&offset,dist_code_len);
					u32 dist=start_dist_lut[dist_code];
					u32 extra_bits_dist=extra_bits_dist_lut[dist_code];
					if(extra_bits_dist)
					{
						dist+=W0&((1<<extra_bits_dist)-1);
						W0=update_word_by_offset(&buffer,&offset,extra_bits_dist);
					}
					// memcpy_s(out_arr+place,0,out_arr+place-dist,length);
					for(u32 i=0;i<length;i++)
					{
						out_arr[place]=out_arr[place-dist];
						place++;
					}

				}
				else
				{
					out_arr[place++]=(u8)symbol;
				}
			}
		}
	}while(BFINAL == 0);
	u32 s1=1,s2=0;
	u32 s1tag = 1, s2tag = 0;
	for(umo i=0;i<place;i++)
	{
		s1=(s1+out_arr[i]);
		s2=(s2+s1);
		// s1tag = (s1tag + out_arr[i]) % (65521u);
		// s2tag = (s2tag + s1tag) % (65521u);
		if((i&((1<<10)-1))==0)
		{
			s1=s1%(65521u);
			s2=s2%(65521u);
		}
	}
	s1 = s1 % (65521u);
	s2 = s2 % (65521u);
	if(offset>0)
	{
		offset=0;
		buffer=(void*)((umo)buffer+1);
	}
	u32 adler=*(u32*)buffer;
	u32 s11 = s1 & 0xff;
	u32 s12 = (s1>>8) & 0xff;
	u32 s21 = s2 & 0xff;
	u32 s22 = (s2 >> 8) & 0xff;
	Assert(adler == ((s11 << 24) | (s12 << 16) | (s21 << 8) | (s22)));
	offset = 0;
	
}
#define CHUNK_NUMBER_IN_REGION 1024
static void* read_and_parse_region_buffer(void* buffer, int length,int x,int z,MemoryBuffer* temp_buffer)
{
	void* start_buffer=buffer;
	umo chunk_offset[CHUNK_NUMBER_IN_REGION];
	umo chunk_length[CHUNK_NUMBER_IN_REGION];
	out_arr=push_array(temp_buffer,u8,1600000);
	Assert(x<32&&z<32);
	int i=x|(z<<5);
	buffer=(void*)((umo)buffer+4*i);
	umo location=read_n_bytes_BE(&buffer,3);
	Assert(location < (u32)length/4096);
	chunk_offset[i]=location;
	chunk_length[i]=read_byte(&buffer);
	buffer=(void*)((umo)start_buffer+8192);
	void* buffer_compressed_section=buffer;

	if(chunk_offset[i]!=0)
	{
		es_memset(out_arr,0,1600000);
		void* chunk_buffer=((u8*)buffer_compressed_section)+(chunk_offset[i]-2)*4096;
		umo chunk_length = read_n_bytes_BE(&chunk_buffer, 4);
		u32 compression = read_byte(&chunk_buffer);
		read_and_parse_zlib_buffer(chunk_buffer,chunk_length-1);//minus 1 is due to the compression being part of the length
		chunk_buffer = chunk_buffer;
	}
	else
	{
		return 0;
	}
	return out_arr;
}
