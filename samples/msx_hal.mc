namespace MSX {
	
	namespace BIOS {
		function[void(int8<register=a>)<asm, address = 0x00a2>] CHPUT;
	};
	
	namespace graphics {
		
		void print( string msg ) {
			
			foreach ( c : msg ) 
				BIOS::CHPUT(c);

			{
				auto begin = msg.begin;
				auto end = msg.end;
				for ( ; begin != end ; ++begin )  {
					auto c = begin[0];	
					BIOS::CHPUT(c);
				}
			}

			{
				typeof(msg.begin) begin = msg.begin;
				typeof(msg.end) end = msg.end;
				for ( ; begin != end ; ++begin )  {
					typeof(begin[0]) c = begin[0];	
					BIOS::CHPUT(c);
				}
			}
		}
	}
}
