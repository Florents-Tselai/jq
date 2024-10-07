# jqllm

`jqllm` does what you think it does:
It gives LLM superpowers to mighty `jq`.

## Generation

```shell
echo '"Tell me a story"' | jqllm 'ollama_generate("llama3.2")'
```

Generate multiple prompts by applying `ollama_generate`
for each element.
Note that this will send separates requests

```shell
echo '[
"Good Evening", 
"Καλησπερα",
"Tell me a story",
"Tell me a story"
]' | jqllm '.[] | ollama_generate("llama3.2")'
```

## Embeddings

### Single

```shell
echo '"Hello"' | jqllm 'ollama_embed("all-minilm")'
```

### Multiple

```shell
echo '[
"Good Evening", 
"Καλησπερα"
]' | jqllm 'ollama_embed("all-minilm")'
```

## Installation

#### Dependencies

- libtool
- make
- automake
- autoconf
- libcurl

#### Instructions

```shell
export CPPFLAGS="$(curl-config --cflags)" 
export LDFLAGS="$(curl-config --libs)" 
export LIBS=-lcurl
```

```console
git submodule update --init    # if building from git to get oniguruma
autoreconf -i                  # if building from git
./configure \
--with-oniguruma=builtin \
--prefix=$(pwd)/build \
--program-suffix=llm

make clean                     # if upgrading from a version previously built from source
make -j8
make check
sudo make install
```