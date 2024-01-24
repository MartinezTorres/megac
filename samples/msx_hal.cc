namespace MSX {
	
	namespace BIOS {
		function<asm, address = 0x00a2>[void(int8_t<register=a>)] CHPUT;
	};
	
	namespace graphics {
		
		void print( string msg ) {
			
			for (c : msg) 
				BIOS::CHPUT(c);
		}
	}
}
