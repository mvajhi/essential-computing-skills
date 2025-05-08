nnoremap <leader>w :w<CR>
nnoremap <leader>v :vsp<CR>
nnoremap <leader>h :sp<CR>

set relativenumber

set ignorecase

set hls

set tabstop=4

syntax on
colorscheme slate

call plug#begin()
  Plug 'tpope/vim-surround'
  Plug 'preservim/nerdtree'
call plug#end()
nnoremap <leader>n :NERDTreeFocus<CR>
nnoremap <C-n> :NERDTree<CR>
nnoremap <C-t> :NERDTreeToggle<CR>
nnoremap <C-f> :NERDTreeFind<CR>
