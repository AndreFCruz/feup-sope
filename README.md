# FEUP-SOPE
___

## Proj1
A parte de programação do trabalho consiste na escrita de uma versão (muito) simplificada do utilitário de Unix "find" que percorre de forma recursiva um diretório, encontrando ficheiros que satisfaçam a algumas condições e sobre eles executando as ações solicitadas.
O programa aqui desenvolvido, sfind, deve ser capaz de reproduzir os resultados apresentados pela invocação de find para os mesmos argumentos. 

## Proj2
Pretende-se desenvolver um programa de simulação de acesso a um recurso partilhado. O recurso é uma Sauna unisexo e o programa será uma simulação informática da sua utilização.
O acesso à sauna é controlado um processo que atende pedidos identificados pelo género do cliente e pelo tempo estimado de duração da ocupação. A sauna tem um certo número de lugares disponíveis, que só podem ser ocupados por clientes do mesmo género. Após a admissão de um utilizador à sauna, o programa controla-lhe o tempo de utilização, libertando depois o lugar para outro utilizador.
A simulação de pedidos é efectuada por um programa gerador que sucessivamente emite pedidos caracterizados por um género e por um tempo de ocupação, ambos gerados aleatoriamente. No caso em que um pedido de acesso não pode ser satisfeito de imediato – por exemplo quando o pedido for masculino, M, e todos os lugares da sauna estiverem ocupados por utilizadores femininos, F –, o pedido é rejeitado e "devolvido" ao gerador que o recolocará na fila de pedidos de acesso.
Todos os pedidos e acessos à sauna são registados em ficheiros, por forma a se poder posteriormente avaliar da correcta execução da simulação.
