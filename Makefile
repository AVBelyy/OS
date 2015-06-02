all: cat/cat revwords/revwords filter/filter bufcat/bufcat simplesh/simplesh filesender/filesender bipiper/forking

cat/cat revwords/revwords filter/filter bufcat/bufcat simplesh/simplesh filesender/filesender bipiper/forking:
	$(MAKE) -C $(dir $@) $(notdir $@)
