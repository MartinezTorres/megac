namespace MSX {
	
	namespace BIOS {
		function[void(int8<register=a>)<asm, address = 0x00a2>] CHPUT;
	};
	
	namespace graphics {
		
		void print( string_view msg ) {
			
			foreach ( c : msg ) 
				BIOS::CHPUT(+c);
		}
	}
}
