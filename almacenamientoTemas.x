struct respTextos{
	int textos;
	string texto<10249>;
};


program ALMACENAMIENTOTEMAS {
	version ALMACENAMIENTO_V1 {
		int init(int i) = 1;
		int putPair(string tema <>, string texto <>) = 2;
		respTextos getTexts(string tema<>) = 3;
	} = 1;
} = 92;
