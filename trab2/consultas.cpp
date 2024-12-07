void buscarProdutoPorCategoriaEMarca(HashTable& hashTable, int categoria, const string& marca, const string& arquivoDados) {
    try {
        // Procura produtos na categoria
        HashNode* resultado = hashTable.search(categoria);
        if (!resultado) {
            cout << "Nenhum produto encontrado para a categoria " << categoria << ".\n";
            return;
        }

        // Abre o arquivo de dados
        ifstream file(arquivoDados, ios::binary);
        if (!file.is_open()) {
            throw runtime_error("Erro ao abrir o arquivo de dados.");
        }

        cout << "Produtos na categoria " << categoria << " da marca \"" << marca << "\":\n";

        // Itera pelos produtos na categoria
        bool encontrado = false;
        while (resultado) {
            file.seekg(resultado->fileIndex, ios::beg);
            Produto produto;
            file.read(reinterpret_cast<char*>(&produto), sizeof(Produto));

            // Verifica a marca do produto
            if (produto.brand == marca) {
                encontrado = true;
                cout << " - Produto: " << produto.brand << " (Chave: " << produto.chave << ", Preço: " << produto.price << ")\n";
            }

            resultado = resultado->next;
        }

        if (!encontrado) {
            cout << "Nenhum produto encontrado com a marca \"" << marca << "\" na categoria " << categoria << ".\n";
        }

        file.close();
    } catch (const exception& e) {
        cerr << "Erro: " << e.what() << endl;
    }
}
