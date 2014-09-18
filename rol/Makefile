# vim: ft=make ff=unix fenc=utf-8
# file: Makefile

all: server

check:
	@find . ! -path '*/.env/*' -type f -name '*.py'\
		-exec sh -c 'pep8 "{}"; pyflakes "{}"' \;

clean:
	rm -rf .env
	find . ! -path '*/.env/*' -type d -name '__pycache__' -exec rm -rf '{}' +
	find . ! -path '*/.env/*' -type f -name '*.pyc' -delete

server: main.py .env/bin/activate .env/requirements
	. .env/bin/activate && python main.py ${o}

.env/bin/activate:
	@echo "Prerpare env"
	@virtualenv -p python3 .env || (rm -rf .env && exit 1)

.env/requirements: requirements.txt
	@echo "Run pip"
	@mkdir -p .env
	@mkdir -p /tmp/pip_dcache
	@. .env/bin/activate\
		&& pip -v install -I --download-cache /tmp/pip_dcache -r requirements.txt
	@cp $< $@

